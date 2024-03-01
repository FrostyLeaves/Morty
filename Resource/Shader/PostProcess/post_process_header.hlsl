#ifndef _POST_PROCESS_HEADER_H_
#define _POST_PROCESS_HEADER_H_
#include "../Internal/internal_uniform_global.hlsl"


struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};


#endif