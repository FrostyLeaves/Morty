#include "privateHeader.hlsl"

struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

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
    Texture2D texDiffuse;
    Texture2D texNormal;
    Texture2D texSpecular;
    Texture2D texTransparent;
    float3 f3Ambient;
    float fAlphaFactor;
    float3 f3Diffuse;
    bool bUseNormalTex;
    float3 f3Specular;
    float fShininess;
    bool bUseSpecularTex;
    bool bUseTransparentTex;
};

//PS
[[vk::binding(0,0)]]cbuffer cbMaterial
{
    Material U_mat;
};




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