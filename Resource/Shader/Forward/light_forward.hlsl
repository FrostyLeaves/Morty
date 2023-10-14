#include "../Internal/internal_uniform_global.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Internal/internal_uniform_model.hlsl"
#include "../Shadow/shadow.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    
#if MCALC_NORMAL_IN_VS
    float3 normal : NORMAL;
    float3 dirLightDirTangentSpace : DIRLIGHT_TANGENT;
    float3 toCameraDirTangentSpace : CAMERADIR_TANGENT;

    float3 pointLightDirTangentSpace[MPOINT_LIGHT_MAX_NUMBER] : POINTLIGHT_TANGENT;
    float3 spotLightDirTangentSpace[MSPOT_LIGHT_MAX_NUMBER] : SPOTLIGHT_TANGENT;
#else
    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;
#endif 

    float3 vertexPointLight : VERTEXX_POINT_LIGHT;

    float3 worldPos : POSITION;
};


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
    Material u_xMaterial;
};

[[vk::binding(1,0)]]Texture2D u_texDiffuse;
[[vk::binding(2,0)]]Texture2D u_texNormal;
[[vk::binding(3,0)]]Texture2D u_texSpecular;
[[vk::binding(4,0)]]Texture2D u_texTransparent;
[[vk::binding(5,0)]]Texture2D u_texEmissive;



// point light
float3 AdditionPointLight(PointLight pointLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3DiffColor, float3 f3SpecColor)
{
    SurfaceData pointData;

    float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(pointData.f3CameraDir, fReflectDir), 0.0f), u_xMaterial.fShininess);

    float fDistance = length(pointLight.f3WorldPosition - f3WorldPixelPosition);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    return float3(     pointLight.f3Intensity * u_xMaterial.f3Diffuse * fDiff * f3DiffColor
                     + pointLight.f3Intensity * u_xMaterial.f3Specular * fSpec * f3SpecColor
                ) * fAttenuation;
}

// spot light
float3 AdditionSpotLight(SpotLight spotLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3DiffColor, float3 f3SpecColor)
{
    float fTheta = dot(spotLight.f3Direction, -f3LightDir);
    if (fTheta > spotLight.fHalfOuterCutOff)
    {
        float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);
        float3 fReflectDir = reflect(-f3LightDir, f3Normal);
        float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), u_xMaterial.fShininess);

        float fEpsilon = spotLight.fHalfInnerCutOff - spotLight.fHalfOuterCutOff;
        float fIntensity = clamp((fTheta - spotLight.fHalfOuterCutOff) / fEpsilon, 0.0, 1.0);

        return float3(     spotLight.f3Intensity * u_xMaterial.f3Diffuse * fDiff * f3DiffColor
                        + spotLight.f3Intensity * u_xMaterial.f3Specular * fSpec * f3SpecColor
                    ) * fIntensity;
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

// direction light
float3 AdditionDirectionLight(float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3DiffColor, float3 f3SpecColor)
{
    float fNdotL = dot(f3Normal, -f3LightDir);

    if (fNdotL >= 0)
    {
        float3 fReflectDir = reflect(f3LightDir, f3Normal);
        float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), u_xMaterial.fShininess);

        return float3(     u_xDirectionalLight.f3Intensity * u_xMaterial.f3Diffuse * fNdotL * f3DiffColor
                        + u_xDirectionalLight.f3Intensity * u_xMaterial.f3Specular * fSpec * f3SpecColor
                    );
    }
    else
    {
        return float3(0, 0, 0);
    }
}

struct LightBasicInfo
{
    float3 f3Normal;
    float3 f3CameraDir;
    float3 f3DirLightDir;
};

LightBasicInfo GetLightBasicInfo(VS_OUT input)
{
    LightBasicInfo result;
    result.f3Normal = float3(0.0f, 0.0f, -1.0f);
    result.f3CameraDir = float3(0.0f, 0.0f, 1.0f);
    result.f3DirLightDir = float3(0.0f, 0.0f, 1.0f);
    

    if (u_xMaterial.bUseNormalTex > 0)
    {
        result.f3Normal = u_texNormal.Sample(LinearSampler, input.uv).xyz;
        result.f3Normal = result.f3Normal.rgb * 2.0f - 1.0f;
        result.f3Normal = normalize(result.f3Normal);
        
//VS Normal
#if MCALC_NORMAL_IN_VS
        
        result.f3CameraDir = input.toCameraDirTangentSpace;
        result.f3DirLightDir = input.dirLightDirTangentSpace;
#else

        float3 T = normalize(input.tangent);
        float3 B = normalize(input.bitangent);
        float3 N = normalize(input.normal);
        float3x3 TBN = float3x3(T,B,N);

        result.f3Normal = mul(result.f3Normal, TBN);
        result.f3CameraDir = normalize(u_f3CameraPosition - input.worldPos);
        result.f3DirLightDir = u_xDirectionalLight.f3LightDir;
#endif
    }
    else
    {
        result.f3Normal = input.normal;
        result.f3CameraDir = normalize(u_f3CameraPosition - input.worldPos);
        result.f3DirLightDir = u_xDirectionalLight.f3LightDir;
    }

    return result;
}

float3 AdditionAllLights(float3 f3Color, float4 f3AmbiColor, VS_OUT input)
{
    LightBasicInfo cLightInfo = GetLightBasicInfo(input);

    float3 f3DiffColor = f3AmbiColor.xyz;
    float3 f3SpecColor = f3AmbiColor.xyz;

    if (u_xMaterial.bUseSpecularTex > 0)
        f3SpecColor = u_texSpecular.Sample(LinearSampler, input.uv).xyz;

    if(u_bDirectionLightEnabled > 0)
    {
        float shadow = GetDirectionShadow(u_texShadowMap, input.worldPos, cLightInfo.f3Normal, -cLightInfo.f3DirLightDir);

        f3Color += shadow * AdditionDirectionLight(  cLightInfo.f3CameraDir,
                                        cLightInfo.f3DirLightDir,
                                        cLightInfo.f3Normal,
                                        f3DiffColor,
                                        f3SpecColor
                                    );
    }
    
    f3Color += input.vertexPointLight;

    for(int nPointLightIdx = 0; nPointLightIdx < min(MPOINT_LIGHT_PIXEL_NUMBER, u_nValidPointLightsNumber); ++nPointLightIdx)
    {

#if MCALC_NORMAL_IN_VS
        float3 f3LightDir = input.pointLightDirTangentSpace[nPointLightIdx];
#else
        float3 f3LightDir = normalize(u_vPointLights[nPointLightIdx].f3WorldPosition - input.worldPos);
#endif
        f3Color += AdditionPointLight(  u_vPointLights[nPointLightIdx],
                                    cLightInfo.f3CameraDir,
                                    f3LightDir,
                                    cLightInfo.f3Normal,
                                    input.worldPos,
                                    f3DiffColor,
                                    f3SpecColor
                                );
    }

    for(int nSpotLightIdx = 0; nSpotLightIdx < min(MSPOT_LIGHT_PIXEL_NUMBER, u_nValidSpotLightsNumber); ++nSpotLightIdx)
    {

#if MCALC_NORMAL_IN_VS
        float3 f3LightDir = input.spotLightDirTangentSpace[nSpotLightIdx];
#else
        float3 f3LightDir = normalize(u_vSpotLights[nSpotLightIdx].f3WorldPosition - input.worldPos);
#endif
        f3Color += AdditionSpotLight(   u_vSpotLights[nSpotLightIdx],
                                    cLightInfo.f3CameraDir,
                                    f3LightDir,
                                    cLightInfo.f3Normal,
                                    input.worldPos,
                                    f3DiffColor,
                                    f3SpecColor);
    }

    return f3Color;
}