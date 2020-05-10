#if MTRANSPARENT_POLICY == 1
#define MTRANSPARENT_DEPTH_PEELING
#endif


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

struct VS_IN_COLOR
{
    float3 pos : POSITION;
    float4 color : COLOR;
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

struct DirectionLight
{
    float3 f3Diffuse;
    float3 f3Specular;
};

struct PointLight
{
    float3 f3WorldPosition;

    float3 f3Diffuse;
    float3 f3Specular;

    float fConstant;
    float fLinear;
    float fQuadratic;

};

struct SpotLight
{
    float3 f3WorldPosition;
    float fHalfInnerCutOff;
    float3 f3Direction;
    float fHalfOuterCutOff;
    float3 f3Diffuse;
    float3 f3Specular;
};

sampler U_defaultSampler : register(s0);
SamplerComparisonState U_shadowMapSampler : register(s1);

//Shadowmap
Texture2D U_texShadowMap : register(t0);

#ifdef MTRANSPARENT_DEPTH_PEELING
//Depth-Peeling
Texture2D U_texDepthFront : register(t1);
#endif

//VS    per mesh
cbuffer _M_E_cbMeshMatrix : register(b0)
{
    float4x4 U_matWorld;
    float3x3 U_matNormal;
};

//VS    per render
cbuffer _M_E_cbWorldMatrix : register(b1)
{
    float4x4 U_matCamProj;
    float4x4 U_matLightProj;
};


//VS    with bones
cbuffer _M_E_cbAnimation : register(b2)
{
    float4x4 U_vBonesMatrix[MBONES_MAX_NUMBER];
};

//PS    per render
cbuffer _M_E_cbLights : register(b3)
{
    DirectionLight U_dirLight;
    PointLight U_pointLights[MPOINT_LIGHT_MAX_NUMBER];
    SpotLight U_spotLights[MSPOT_LIGHT_MAX_NUMBER];
    bool U_bDirectionLightEnabled;
    int U_nValidPointLightsNumber;
    int U_nValidSpotLightsNumber;
};

//VS & PS    per render
cbuffer _M_E_cbWorldInfo : register(b4)
{
    float3 U_f3DirectionLight;
    float3 U_f3CameraPosition;

    float2 U_f2ViewportSize;
  //  float3 U_vPointLightPosition[MPOINT_LIGHT_MAX_NUMBER];
};