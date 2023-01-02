#ifndef _M_INNER_CONSTANT_HLSL_
#define _M_INNER_CONSTANT_HLSL_


#ifndef NUM_PI
    #define NUM_PI (3.1415926535898)
    #define NUM_PI2 (6.283185307179586)
#endif

#define NUM_BIAS (0.000001f)

#if MTRANSPARENT_POLICY == 1 && MEN_TRANSPARENT == 1
#define MTRANSPARENT_DEPTH_PEELING
#endif

struct DirectionLight
{
    float3 f3Intensity;
    float fLightSize;
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
[[vk::binding(0,1)]]cbuffer cbSceneMatrix : register(b1)
{
    float4x4 u_matView; // world to view
    float4x4 u_matCamProj; // world to proj
    float4x4 u_matCamProjInv; // proj to world
};

//VS & PS    per render
[[vk::binding(1,1)]]cbuffer cbSceneInformation : register(b2)
{
    float3 u_f3DirectionLight;
    float3 u_f3CameraPosition;
    float3 u_f3CameraDirection;

    float2 u_f2ViewportSize;
    float2 u_matZNearFar;
    float u_fDelta;
    float u_fGameTime;
};

//PS    per render
[[vk::binding(2,1)]]cbuffer cbLightInformation : register(b3)
{
    DirectionLight u_xDirectionalLight;
    PointLight u_vPointLights[MPOINT_LIGHT_MAX_NUMBER];
    SpotLight u_vSpotLights[MSPOT_LIGHT_MAX_NUMBER];
    int u_bDirectionLightEnabled;
    int u_nValidPointLightsNumber;
    int u_nValidSpotLightsNumber;
    int u_bEnvironmentMapEnabled;
};

[[vk::binding(3,1)]]cbuffer cbShadowInformation : register(b4)
{
    float4x4 u_vLightProjectionMatrix[CASCADED_SHADOW_MAP_NUM];
    float u_vCascadeSplits[CASCADED_SHADOW_MAP_NUM];
};

[[vk::binding(4,1)]]sampler LinearSampler;
[[vk::binding(5,1)]]sampler NearestSampler;

//Shadowmap
[[vk::binding(6,1)]]Texture2DArray u_texShadowMap;
[[vk::binding(7,1)]]TextureCube u_texIrradianceMap;
[[vk::binding(8,1)]]TextureCube u_texPrefilterMap;
[[vk::binding(9,1)]]Texture2D u_texBrdfLUT;


#endif