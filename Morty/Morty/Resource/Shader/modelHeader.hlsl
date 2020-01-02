struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;

    float3 normal : NORMAL;
    float3 worldPos : WORLDPOS;
};

struct Material
{
    Texture2D texDiffuse;
    Texture2D texSpecular;
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

sampler U_defaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

cbuffer cbSpace
{
    float4x4 U_matWorld;
    float4x4 U_matCamProj;
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