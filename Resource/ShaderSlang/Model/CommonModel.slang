implementing ModelModule;

#include "Internal/PreDefine.slang"

public struct VertexIn
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

public struct VertexOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;

    float3 positionWS : WORLD_POS;
    float depth : DEPTH;

    float3 normal : NORMAL;
    float3 tangent : Tangent;
    float3 bitangent : BITANGENT;
    uint instanceId : INSTANCE_ID;
}

public interface IBatchMeshData
{
    public MeshInstanceRenderProxy GetInstanceProxy(uint instanceId);
}

StructuredBuffer<float4x4> u_vBonesMatrix;
StructuredBuffer<int> u_vBonesOffset;

