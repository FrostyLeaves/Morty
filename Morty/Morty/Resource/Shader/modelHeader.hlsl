struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;

    float3 normal : NORMAL;
    float3 worldPos : WORLDPOS;

    float4 lightSpacePos : LIGHTSPACEPOS;
};

struct VS_IN_EMPTY
{
    float3 pos : POSITION;
};

struct VS_IN_ANIM
{
    float3 pos : POSITION;

    int bonesID[MBONES_PER_VERTEX] : BONES_ID;
    float bonesWeight[MBONES_PER_VERTEX] : BONES_WEIGHT;
};

struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

struct Material
{
    Texture2D texDiffuse;
    Texture2D texSpecular;
    Texture2D texShadowMap;
    float3 f3Ambient;
    float3 f3Diffuse;
    float3 f3Specular;
    float fShininess;
};

struct DirectionLight
{
    float3 f3Direction;
    float3 f3Ambient;
    float3 f3Diffuse;
    float3 f3Specular;
};

struct PointLight
{
    float3 f3WorldPosition;

    float3 f3Ambient;
    float3 f3Diffuse;
    float3 f3Specular;

    float fConstant;
    float fLinear;
    float fQuadratic;

};

sampler U_defaultSampler;
SamplerComparisonState U_shadowMapSampler;

cbuffer cbSpace
{
    float4x4 U_matWorld;
    float4x4 U_matCamProj;
    float4x4 U_matLightProj;
    float3x3 U_matNormal;
};

cbuffer cbMaterial
{
    Material U_mat;
};

cbuffer cbLights
{
    DirectionLight U_dirLight;
    PointLight U_pointLights[4];
};

cbuffer cbWorldInfo
{
    float3 U_f3CameraWorldPos;
};

cbuffer cbAnimation
{
    float4x4 U_vBonesMatrix[MBONES_MAX_NUMBER];
};