#include "private_header.hlsl"
#include "model_struct.hlsl"

struct Material
{
    float3 f3Ambient;
    float fAlphaFactor;
    float3 f3Diffuse;
    int bUseNormalTex;
    float3 f3Specular;
    float fShininess;
    int bUseSpecularTex;
    int bUseTransparentTex;
    int bUseEmissiveTex;
};

//PS
[[vk::binding(0,0)]]cbuffer cbMaterial
{
    Material U_mat;
};


//Textures
[[vk::binding(1,0)]]Texture2D U_mat_texAlbedo;
[[vk::binding(2,0)]]Texture2D U_mat_texNormal;
[[vk::binding(3,0)]]Texture2D U_mat_texMetallic;
[[vk::binding(4,0)]]Texture2D U_mat_texRoughness;
[[vk::binding(5,0)]]Texture2D U_mat_texAmbientOcc;

float3 GetNormal()
{
    float3 f3Normal = U_mat_texNormal.Sample(U_defaultSampler, input.uv).xyz;
    f3Normal = f3Normal.rgb * 2.0f - 1.0f;
    f3Normal = normalize(f3Normal);
    
    //使用法线贴图，法向量在view space，CameraDir也在view space

    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3 N = normalize(input.normal);
    float3x3 TBN = float3x3(T,B,N);

    return mul(f3Normal, TBN);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float3 CalcPBRLight(float3 f3LightColor, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BasicColor, float3 fRoughness)
{
    // 视线方向 + 光照方向 的中间向量
    float3 f3HalfDir = normalize(f3CameraDir + f3LightDir);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(f3Normal, f3HalfDir, fRoughness);
    float G   = GeometrySmith(f3Normal, f3CameraDir, f3LightDir, fRoughness);
    float3 F    = FresnelSchlick(max(dot(f3HalfDir, f3CameraDir), 0.0), f3BasicColor);
        
    vec3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(f3Normal, f3CameraDir), 0.0) * max(dot(f3Normal, f3LightDir), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;
    
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    return (kD * albedo / PI + specular) * f3LightColor * NdotL;
}

// spot light
float3 CalcSpotLight(SpotLight spotLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3BasicColor, float fRoughness)
{
    float fTheta = dot(spotLight.f3Direction, -f3LightDir);
    if (fTheta > spotLight.fHalfOuterCutOff)
    {
        float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);
        float3 fReflectDir = reflect(-f3LightDir, f3Normal);
        float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

        float fEpsilon = spotLight.fHalfInnerCutOff - spotLight.fHalfOuterCutOff;
        float fIntensity = clamp((fTheta - spotLight.fHalfOuterCutOff) / fEpsilon, 0.0, 1.0);

        float3 f3LightColor = spotLight.f3Diffuse * fIntensity;
                    
        return CalcPBRLight(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BasicColor, fRoughness);
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

// point light
float3 CalcPointLight(PointLight pointLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BasicColor, float fRoughness)
{
    float fDistance = length(pointLight.f3WorldPosition - input.worldPos);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    float3 f3LightColor = pointLight.f3Diffuse * fAttenuation;

    return CalcPBRLight(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BasicColor, fRoughness);
}

// point light
float3 CalcDirectionLight(float4 f4DirLightSpacePos, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BasicColor, float fRoughness)
{
    float fNdotL = dot(f3Normal, -f3LightDir);

    if (fNdotL >= 0)
    {
        float shadow = CalcShadow(f4DirLightSpacePos, fNdotL);

        float fDistance = length(pointLight.f3WorldPosition - input.worldPos);
        float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

        float3 f3LightColor = pointLight.f3Diffuse * fAttenuation;

        return CalcPBRLight(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BasicColor, fRoughness);
    }

    return float3(0, 0, 0);
}

float3 AdditionAllLights(VS_OUT input, float3 f3Color)
{
    float3 f3Albedo   = pow(U_mat_texAlbedo.Sample(U_defaultSampler, input.uv).rgb, vec3(2.2));
    float fMetallic   = U_mat_texMetallic.Sample(U_defaultSampler, input.uv).r;
    float fRoughness  = U_mat_texRoughness.Sample(U_defaultSampler, input.uv).r;
    //float fAmbientOcc = U_mat_texAmbientOcc.Sample(U_defaultSampler, input.uv).r;

    float3 f3Normal = GetNormal();

    float3 f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    float3 f3BasicColor = mix(float3(0.04), f3Albedo, fMetallic);



    LightBasicInfo cLightInfo = GetLightBasicInfo(input);

    if(U_bDirectionLightEnabled > 0)
    {
        f3Color += CalcDirectionLight(  input.dirLightSpacePos,
                                        cLightInfo.f3CameraDir,
                                        cLightInfo.f3DirLightDir,
                                        cLightInfo.f3Normal,
                                        f3BasicColor
                                    );
    }
    
    f3Color += input.vertexPointLight;

    for(int i = 0; i < min(MPOINT_LIGHT_PIXEL_NUMBER, U_nValidPointLightsNumber); ++i)
    {
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        float3 f3LightDir = input.pointLightDirTangentSpace[i];
#else
        float3 f3LightDir = normalize(U_pointLights[i].f3WorldPosition - input.worldPos);
#endif

        f3Color += CalcPointLight(  U_pointLights[i],
                                    cLightInfo.f3CameraDir,
                                    f3LightDir,
                                    cLightInfo.f3Normal,
                                    f3BasicColor
                                );
    }

    for(int i = 0; i < min(MSPOT_LIGHT_PIXEL_NUMBER, U_nValidSpotLightsNumber); ++i)
    {
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        float3 f3LightDir = input.spotLightDirTangentSpace[i];
#else
        float3 f3LightDir = normalize(U_spotLights[i].f3WorldPosition - input.worldPos);
#endif
        f3Color += CalcSpotLight(   U_spotLights[i],
                                    cLightInfo.f3CameraDir,
                                    f3LightDir,
                                    cLightInfo.f3Normal,
                                    input.worldPos,
                                    f3BasicColor);
    }

    return f3Color;
}