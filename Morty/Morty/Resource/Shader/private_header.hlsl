#if MTRANSPARENT_POLICY == 1 && MEN_TRANSPARENT == 1
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

#if SKELETON_ENABLE == 1
    int bonesID[MBONES_PER_VERTEX] : BONES_ID;
    float bonesWeight[MBONES_PER_VERTEX] : BONES_WEIGHT;
#endif

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

float4 FloatToFloat4(float depth)
{
    const float4 bit_shift = float4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const float4 bit_mask  = float4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    float4 res = frac(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return res;
}

float Float4ToFloat(float4 rgba_depth)
{
    const float4 bit_shift = float4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth, bit_shift);
    return depth;
}


//VS    per render
[[vk::binding(0,1)]]cbuffer _M_E_cbWorldMatrix : register(b1)
{
    float4x4 U_matCamProj;
    float4x4 U_matLightProj;
};

//VS & PS    per render
[[vk::binding(1,1)]]cbuffer _M_E_cbWorldInfo : register(b4)
{
    float3 U_f3DirectionLight;
    float3 U_f3CameraPosition;

    float2 U_f2ViewportSize;
};

//PS    per render
[[vk::binding(2,1)]]cbuffer _M_E_cbLights : register(b3)
{
    DirectionLight U_dirLight;
    PointLight U_pointLights[MPOINT_LIGHT_MAX_NUMBER];
    SpotLight U_spotLights[MSPOT_LIGHT_MAX_NUMBER];
    int U_bDirectionLightEnabled;
    int U_nValidPointLightsNumber;
    int U_nValidSpotLightsNumber;
};

[[vk::binding(3,1)]]sampler U_defaultSampler : register(s0);
[[vk::binding(4,1)]]SamplerComparisonState U_lessEqualSampler : register(s1);
[[vk::binding(5,1)]]SamplerComparisonState U_greaterEqualSampler : register(s2);

//Shadowmap
[[vk::binding(6,1)]]Texture2D U_texShadowMap : register(t0);
