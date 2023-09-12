#ifndef _M_VOXELIER_STRUCT_HLSL_
#define _M_VOXELIER_STRUCT_HLSL_

#include "../Internal/internal_constant.hlsl"
#include "../Internal/internal_model.hlsl"
#include "../Deferred/pbr_material.hlsl"
#include "../Voxel/voxel_struct_define.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
	float3 normal : NORMAL;
};


struct PS_OUT
{
    float4 f4Color: SV_Target0;
};

struct VoxelizerOutput
{
    uint nVoxelCount;
};


[[vk::binding(10,1)]] cbuffer cbVoxelMap
{
    VoxelMapSetting voxelMapSetting;
};

[[vk::binding(11,1)]] RWStructuredBuffer<VoxelizerOutput> rwVoxelTable;


int3 getVoxelMapUVW(float3 f3WorldPosition)
{
    int3 uvw = int3((f3WorldPosition - voxelMapSetting.f3VoxelOrigin) / voxelMapSetting.fVoxelStep);

    return uvw;
}



#endif