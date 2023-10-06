#ifndef _M_VOXELIER_STRUCT_HLSL_
#define _M_VOXELIER_STRUCT_HLSL_

#include "Internal/internal_constant.hlsl"
#include "Internal/internal_model.hlsl"
#include "Internal/internal_functional.hlsl"
#include "Deferred/pbr_material.hlsl"


struct VS_OUT
{
    float4 pos : SV_POSITION;
	float3 normal : NORMAL;
    float3 worldPos : WORLD_POSITION;
};

struct GS_OUT
{
    
};

struct PS_OUT
{
    float4 f4Color: SV_Target;
};



#endif