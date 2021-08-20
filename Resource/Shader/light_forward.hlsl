#include "model_struct.hlsl"

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

    float4 dirLightSpacePos : DIRLIGHTSPACEPOS;
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
    Material U_mat;
};

[[vk::binding(1,0)]]Texture2D U_mat_texDiffuse;
[[vk::binding(2,0)]]Texture2D U_mat_texNormal;
[[vk::binding(3,0)]]Texture2D U_mat_texSpecular;
[[vk::binding(4,0)]]Texture2D U_mat_texTransparent;
[[vk::binding(5,0)]]Texture2D U_mat_texEmissive;



// point light
float3 CalcPointLight(PointLight pointLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3DiffColor, float3 f3SpecColor)
{

    float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    float fDistance = length(pointLight.f3WorldPosition - f3WorldPixelPosition);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    return float3(     pointLight.f3Diffuse * U_mat.f3Diffuse * fDiff * f3DiffColor
                     + pointLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                ) * fAttenuation;
}

// spot light
float3 CalcSpotLight(SpotLight spotLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3DiffColor, float3 f3SpecColor)
{
    float fTheta = dot(spotLight.f3Direction, -f3LightDir);
    if (fTheta > spotLight.fHalfOuterCutOff)
    {
        float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);
        float3 fReflectDir = reflect(-f3LightDir, f3Normal);
        float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

        float fEpsilon = spotLight.fHalfInnerCutOff - spotLight.fHalfOuterCutOff;
        float fIntensity = clamp((fTheta - spotLight.fHalfOuterCutOff) / fEpsilon, 0.0, 1.0);

        return float3(     spotLight.f3Diffuse * U_mat.f3Diffuse * fDiff * f3DiffColor
                        + spotLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                    ) * fIntensity;
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

// direction light
float3 CalcDirectionLight(float4 f4DirLightSpacePos, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3DiffColor, float3 f3SpecColor)
{
    float fNdotL = dot(f3Normal, -f3LightDir);

    if (fNdotL >= 0)
    {
        //float shadow = CalcShadow(U_texShadowMap, f4DirLightSpacePos, fNdotL);
        float shadow = 1.0f;
        float3 fReflectDir = reflect(f3LightDir, f3Normal);
        float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

        return float3(     U_dirLight.f3Diffuse * U_mat.f3Diffuse * fNdotL * f3DiffColor
                        + U_dirLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                    ) * shadow;
    }
    else
    {
        return float3(0, 0, 0);
    }
}

LightBasicInfo GetLightBasicInfo(VS_OUT input)
{
    LightBasicInfo result;
    result.f3Normal = float3(0.0f, 0.0f, -1.0f);
    result.f3CameraDir = float3(0.0f, 0.0f, 1.0f);
    result.f3DirLightDir = float3(0.0f, 0.0f, 1.0f);
    

    if (U_mat.bUseNormalTex > 0)
    {
        result.f3Normal = U_mat_texNormal.Sample(U_defaultSampler, input.uv).xyz;
        result.f3Normal = result.f3Normal.rgb * 2.0f - 1.0f;
        result.f3Normal = normalize(result.f3Normal);
        
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        //使用法线贴图， 法向量在切线空间， CameraDir也在切线空间
        
        result.f3CameraDir = input.toCameraDirTangentSpace;
        result.f3DirLightDir = input.dirLightDirTangentSpace;
#else
        //使用法线贴图，法向量在view space，CameraDir也在view space

        float3 T = normalize(input.tangent);
        float3 B = normalize(input.bitangent);
        float3 N = normalize(input.normal);
        float3x3 TBN = float3x3(T,B,N);

        result.f3Normal = mul(result.f3Normal, TBN);
        result.f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);
        result.f3DirLightDir = U_f3DirectionLight;
#endif
    }
    else
    {
        //没用法线贴图，法向量在view space，CameraDir也在view space
        result.f3Normal = input.normal;
        result.f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);
        result.f3DirLightDir = U_f3DirectionLight;
    }

    return result;
}

float3 AdditionAllLights(float3 f3Color, float4 f3AmbiColor, VS_OUT input)
{
    LightBasicInfo cLightInfo = GetLightBasicInfo(input);

    float3 f3DiffColor = f3AmbiColor.xyz;
    float3 f3SpecColor = f3AmbiColor.xyz;

    if (U_mat.bUseSpecularTex > 0)
        f3SpecColor = U_mat_texSpecular.Sample(U_defaultSampler, input.uv).xyz;

    if(U_bDirectionLightEnabled > 0)
    {
        f3Color += CalcDirectionLight(  input.dirLightSpacePos,
                                        cLightInfo.f3CameraDir,
                                        cLightInfo.f3DirLightDir,
                                        cLightInfo.f3Normal,
                                        f3DiffColor,
                                        f3SpecColor
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
                                    input.worldPos,
                                    f3DiffColor,
                                    f3SpecColor
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
                                    f3DiffColor,
                                    f3SpecColor);
    }

    return f3Color;
}