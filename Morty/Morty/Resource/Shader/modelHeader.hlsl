struct VS_OUT_EMPTY
{
    float4 pos : SV_POSITION;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;

    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;
    
    float3 worldPos : WORLDPOS;

    float4 lightSpacePos : LIGHTSPACEPOS;
};

struct VS_IN_EMPTY
{
    float3 pos : POSITION;
};

struct VS_IN_EMPTY_ANIM
{
    float3 pos : POSITION;

    int bonesID[MBONES_PER_VERTEX] : BONES_ID;
    float bonesWeight[MBONES_PER_VERTEX] : BONES_WEIGHT;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : TEXCOORDS;
};

struct VS_IN_ANIM
{
    float3 pos : POSITION;

    int bonesID[MBONES_PER_VERTEX] : BONES_ID;
    float bonesWeight[MBONES_PER_VERTEX] : BONES_WEIGHT;
    
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : TEXCOORDS;

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

//VS
cbuffer MORTY_ENGINE_cbMeshMatrix
{
    float4x4 U_matWorld;
    float3x3 U_matNormal;
};

cbuffer MORTY_ENGINE_cbWorldMatrix
{
    float4x4 U_matCamProj;
    float4x4 U_matLightProj;
};

//VS
cbuffer MORTY_ENGINE_cbAnimation
{
    float4x4 U_vBonesMatrix[MBONES_MAX_NUMBER];
};

//PS
cbuffer cbMaterial
{
    Material U_mat;
    Texture2D U_texShadowMap;
};

//PS
cbuffer MORTY_ENGINE_cbLights
{
    DirectionLight U_dirLight;
    PointLight U_pointLights[4];
};

//PS
cbuffer MORTY_ENGINE_cbWorldInfo
{
    float3 U_f3CameraPosition;
};
