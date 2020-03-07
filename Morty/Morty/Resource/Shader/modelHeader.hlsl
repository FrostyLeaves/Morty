#include "privateHeader.hlsl"

struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;

#if MCALC_NORMAL_IN_VS
    float3 normal : NORMAL;
    float3 dirLightDirTangentSpace : DIRLIGHT_TANGENT;
    float3 toCameraDirTangentSpace : CAMERADIR_TANGENT;
#else
    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;
#endif 

    float3 worldPos : WORLDPOS;

    float4 dirLightSpacePos : DIRLIGHTSPACEPOS;
};

struct Material
{
    Texture2D texDiffuse;
    Texture2D texNormal;
    Texture2D texSpecular;
    float3 f3Ambient;
    float3 f3Diffuse;
    float3 f3Specular;
    float fShininess;
    bool bUseNormalTex;
};

//PS
cbuffer cbMaterial
{
    Material U_mat;
};
