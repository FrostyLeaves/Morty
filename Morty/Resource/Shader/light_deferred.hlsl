#include "inner_constant.hlsl"
#include "inner_functional.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

//Textures
[[vk::binding(0,0)]]Texture2D U_mat_f3Base_fMetal;
[[vk::binding(1,0)]]Texture2D U_mat_f3Albedo_fAmbientOcc;
[[vk::binding(2,0)]]Texture2D U_mat_f3Normal_fRoughness;
[[vk::binding(3,0)]]Texture2D U_mat_fDepth;


float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = NUM_PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
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

float3 CalcPBRLight(float3 f3LightColor, float3 f3CameraDir, float3 _f3LightDir, float3 f3Normal, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float3 f3LightDir = -_f3LightDir;

    // 视线方向 + 光照方向 的中间向量
    float3 f3HalfDir = normalize(f3CameraDir + f3LightDir);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(f3Normal, f3HalfDir, fRoughness);
    float G   = GeometrySmith(f3Normal, f3CameraDir, f3LightDir, fRoughness);
    float3 F  = FresnelSchlick(max(dot(f3HalfDir, f3CameraDir), 0.0), f3BaseColor);
        
    float3 nominator    = NDF * G * F; 
    float denominator = 4 * max(dot(f3Normal, f3CameraDir), 0.0) * max(dot(f3Normal, f3LightDir), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    float3 specular = nominator / denominator;
    
    // kS is equal to Fresnel
    float3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    float3 kD = float3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= float3(1.0) - fMetallic;	  

    // scale light by NdotL
    float NdotL = max(dot(f3Normal, f3LightDir), 0.0);        

    // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    return (kD * f3Albedo / NUM_PI + specular) * f3LightColor * NdotL;
}

// spot light
float3 CalcSpotLight(SpotLight spotLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float fTheta = dot(spotLight.f3Direction, -f3LightDir);
    if (fTheta > spotLight.fHalfOuterCutOff)
    {
        float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);
        float3 fReflectDir = reflect(-f3LightDir, f3Normal);

        float fEpsilon = spotLight.fHalfInnerCutOff - spotLight.fHalfOuterCutOff;
        float fIntensity = clamp((fTheta - spotLight.fHalfOuterCutOff) / fEpsilon, 0.0, 1.0);

        float3 f3LightColor = spotLight.f3Diffuse * fIntensity;
                    
        return CalcPBRLight(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic);
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

// point light
float3 CalcPointLight(PointLight pointLight, float3 worldPos, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float fDistance = length(pointLight.f3WorldPosition - worldPos);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    float3 f3LightColor = pointLight.f3Diffuse * fAttenuation;

    return CalcPBRLight(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic);
}

// point light
float3 CalcDirectionLight(DirectionLight dirLight, float4 f4DirLightSpacePos, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3BaseColor, float3 f3Albedo, float fRoughness, float fMetallic)
{
    float fNdotL = dot(f3Normal, -f3LightDir);

    if (fNdotL >= 0)
    {
        float shadow = CalcShadow(U_texShadowMap, f4DirLightSpacePos, fNdotL);

        float3 f3LightColor = dirLight.f3Diffuse;

        return CalcPBRLight(f3LightColor, f3CameraDir, f3LightDir, f3Normal, f3BaseColor, f3Albedo, fRoughness, fMetallic);
    }

    return float3(0, 0, 0);
}

float3 GetWorldPosition(VS_OUT input)
{
    float2 pos = input.uv * 2.0 - 1.0;
    pos.y = 1.0 - pos.y;

    float4 f4DepthColor = U_mat_fDepth.Sample(U_defaultSampler, input.uv);

    float fDepth = Float4ToFloat(f4DepthColor);

    fDepth = fDepth * (U_matZNearFar.y - U_matZNearFar.x) + U_matZNearFar.x;

    float4 f4ViewportToWorldPos = mul(float4(pos.x, pos.y, U_matZNearFar.x, 1.0f), U_matCamProjInv);

    float3 f3WorldPosition = f4ViewportToWorldPos.xyz / f4ViewportToWorldPos.w;

    return U_f3CameraPosition + normalize(f3WorldPosition - U_f3CameraPosition) * fDepth;
}

float3 AdditionAllLights(VS_OUT input, float3 f3Color)
{

    float4 f3Base_fMetal = U_mat_f3Base_fMetal.Sample(U_defaultSampler, input.uv);
    float4 f3Albedo_fAmbientOcc = U_mat_f3Albedo_fAmbientOcc.Sample(U_defaultSampler, input.uv);
    float4 f3Normal_fRoughness = U_mat_f3Normal_fRoughness.Sample(U_defaultSampler, input.uv);

    float3 f3BaseColor = f3Base_fMetal.rgb;
    float3 f3Albedo   = pow(f3Albedo_fAmbientOcc.rgb, float3(2.2));
    float3 f3Normal = f3Normal_fRoughness.rgb;
    f3Normal = normalize(f3Normal * 2.0 - 1.0);

    float fMetallic   = f3Base_fMetal.a;
    float fAmbientOcc = f3Albedo_fAmbientOcc.a;
    float fRoughness = f3Normal_fRoughness.a;

    float3 f3WorldPosition = GetWorldPosition(input);



    float3 f3CameraDir = normalize(U_f3CameraPosition - f3WorldPosition);

//    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
//    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
//    float3 f3BaseColor = lerp(float3(0.04), f3Albedo, fMetallic);

    if(U_bDirectionLightEnabled > 0)
    {
        float3 f3DirLightDir = U_f3DirectionLight;
        float4 f3DirLightSpacePos = mul(float4(f3WorldPosition, 1.0f), U_matLightProj);

        f3Color += CalcDirectionLight(  U_dirLight,
                                        f3DirLightSpacePos,
                                        f3CameraDir,
                                        f3DirLightDir,
                                        f3Normal,
                                        f3BaseColor,
                                        f3Albedo,
                                        fRoughness,
                                        fMetallic
                                    );
    }
    
    for(int i = 0; i < min(MPOINT_LIGHT_PIXEL_NUMBER, U_nValidPointLightsNumber); ++i)
    {
        float3 f3LightDir = normalize(U_pointLights[i].f3WorldPosition - f3WorldPosition);

        f3Color += CalcPointLight(  U_pointLights[i],
                                    f3WorldPosition,
                                    f3CameraDir,
                                    f3LightDir,
                                    f3Normal,
                                    f3BaseColor,
                                    f3Albedo,
                                    fRoughness,
                                    fMetallic
                                );
    }

    for(int i = 0; i < min(MSPOT_LIGHT_PIXEL_NUMBER, U_nValidSpotLightsNumber); ++i)
    {
        float3 f3LightDir = normalize(U_spotLights[i].f3WorldPosition - f3WorldPosition);
        f3Color += CalcSpotLight(   U_spotLights[i],
                                    f3CameraDir,
                                    f3LightDir,
                                    f3Normal,
                                    f3WorldPosition,
                                    f3BaseColor,
                                    f3Albedo,
                                    fRoughness,
                                    fMetallic
                                );
    }

    float3 f3Ambient = float3(0.1) * f3Albedo * fAmbientOcc;
    
    f3Color = f3Color + f3Ambient;

    // HDR tonemapping
    f3Color = f3Color / (f3Color + float3(1.0));
    // gamma correct
    f3Color = pow(f3Color, float3(1.0/2.2)); 

    return f3Color;
}
