#ifndef _M_VOXELIER_STRUCT_HLSL_
#define _M_VOXELIER_STRUCT_HLSL_

#include "../Internal/internal_constant.hlsl"
#include "../Internal/internal_model.hlsl"

struct VS_OUT
{
    float4 pos : SV_POSITION;
	float3 normal : NORMAL;
};


struct VoxelData
{
    uint nVoxelCount;
};




[[vk::binding(0,0)]] cbuffer cbVoxelMapSetting
{
    float3 f3VoxelOrigin;       //voxel map origin position in world space.
    float fResolution;          //voxel table resolution.
    float fVoxelStep;      //how much distance does a voxel.
};


[[vk::binding(1,0)]] RWStructuredBuffer<VoxelData> voxelTable;


#endif