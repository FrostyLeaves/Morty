#ifndef _M_INTERNAL_UNIFORM_GLOBAL_HLSL_
#define _M_INTERNAL_UNIFORM_GLOBAL_HLSL_

#include "../Internal/internal_constant.hlsl"



[[vk::binding(0,1)]]cbuffer cbFrameInformation
{
    //cbSceneMatrix
    float4x4 u_matView; // world to view
    float4x4 u_matCamProj; // world to proj
    float4x4 u_matCamProjInv; // proj to world

    //cbSceneInformation
    float3 u_f3CameraPosition;
    float3 u_f3CameraDirection;

    float2 u_f2ViewportSize;
    float2 u_matZNearFar;
    float u_fDelta;
    float u_fGameTime;

};

//Sampler
[[vk::binding(1,1)]]sampler LinearSampler;
[[vk::binding(2,1)]]sampler NearestSampler;

[[vk::binding(3,1)]]cbuffer cbLightInformation
{
    //cbLightInformation
    DirectionLight u_xDirectionalLight;
    PointLight u_vPointLights[MPOINT_LIGHT_MAX_NUMBER];
    SpotLight u_vSpotLights[MSPOT_LIGHT_MAX_NUMBER];
    int u_bDirectionLightEnabled;
    int u_nValidPointLightsNumber;
    int u_nValidSpotLightsNumber;
    int u_bEnvironmentMapEnabled;

    //cbShadowInformation
    float4x4 u_vLightProjectionMatrix[CASCADED_SHADOW_MAP_NUM];
    //x: cascade split max range
    //y: x * 1.25f
    //z: cascade ortho matrix width
    //w: light position z in projection space.
    float4 u_vCascadeSplits[CASCADED_SHADOW_MAP_NUM];
};

//Shadowmap
[[vk::binding(4,1)]]Texture2DArray u_texShadowMap;

//Environment
[[vk::binding(5,1)]]TextureCube u_texIrradianceMap;
[[vk::binding(6,1)]]TextureCube u_texPrefilterMap;
[[vk::binding(7,1)]]Texture2D u_texBrdfLUT;

//Animation
[[vk::binding(8,1)]] StructuredBuffer<float4x4> u_vBonesMatrix;
[[vk::binding(9,1)]] StructuredBuffer<int> u_vBonesOffset;

//Voxel
#if MORTY_VXGI_ENABLE
[[vk::binding(10,1)]]cbuffer cbVoxelMapInformation
{
    //cbVoxelMap
    VoxelMapSetting voxelMapSetting;
    float4x4 u_m4VoxelizerCamProj;
}

[[vk::binding(11,1)]] RWStructuredBuffer<VoxelizerOutput> u_rwVoxelTable;
[[vk::binding(12,1)]] Texture3D<float4> u_texVoxelMap;
#endif



#endif