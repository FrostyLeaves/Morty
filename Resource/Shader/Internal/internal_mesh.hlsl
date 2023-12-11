#ifndef _M_INTERNAL_MESH_DEFINE_HLSL_
#define _M_INTERNAL_MESH_DEFINE_HLSL_


struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : TEXCOORDS;
    
#if SKELETON_ENABLE
    int bonesID[MBONES_PER_VERTEX] : BONES_ID;
    float bonesWeight[MBONES_PER_VERTEX] : BONES_WEIGHT;
#endif

};


#endif