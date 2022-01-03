
#define NUM_PI (3.1415926535898)
#define NUM_BIAS (0.000001f)

#if MTRANSPARENT_POLICY == 1 && MEN_TRANSPARENT == 1
#define MTRANSPARENT_DEPTH_PEELING
#endif

struct DirectionLight
{
    float3 f3Intensity;
};

struct PointLight
{
    float3 f3WorldPosition;

    float3 f3Intensity;

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
    float3 f3Intensity;
};



//VS    per render
[[vk::binding(0,1)]]cbuffer _M_E_cbWorldMatrix : register(b1)
{
    float4x4 U_matProj;
    float4x4 U_matCamProj; // world to proj
    float4x4 U_matCamProjInv; // proj to world
    float4x4 U_matLightProj;
};

//VS & PS    per render
[[vk::binding(1,1)]]cbuffer _M_E_cbWorldInfo : register(b4)
{
    float3 U_f3DirectionLight;
    float3 U_f3CameraPosition;

    float2 U_f2ViewportSize;
    float2 U_matZNearFar;
    float U_fDelta;
    float U_fGameTime;
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

//Shadowmap
[[vk::binding(6,1)]]Texture2D U_texShadowMap : register(t0);
