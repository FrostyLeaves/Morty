#ifndef _DEFERRED_VSOUT_H_
#define _DEFERRED_VSOUT_H_

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    
    float3 worldPos : WORLD_POS;
    float depth : DEPTH;

    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;

#ifdef VOXELIZER_CONSERVATIVE_RASTERIZATION
    float3 aabbMin : AABB_MIN;
    float3 aabbMax : AABB_MAX;
#endif
};

#endif