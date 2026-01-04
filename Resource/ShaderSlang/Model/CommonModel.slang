implementing ModelModule;

#include "Internal/PreDefine.slang"

public struct ClusterData
{
    uint indexOffset;
    uint indexCount;
    uint valid;

    float3 position;
    float radius;
    float error;
};

public struct ClusterGroupData
{
    uint valid;
    uint parentGroupId;
    uint firstGroupId;
    uint childGroupCount;

    uint clusterBeginIndex;
    uint clusterCount;

    float3 position;
    float radius;
    float error;
};

// Mesh resource level data, used for finding ClusterGroup root nodes during GPU culling
public struct MeshResourceData
{
    int rootClusterGroupBeginIndex;
    int rootClusterGroupCount;
};

public struct MeshInstanceData
{
    public float4x4 matWorld;
    public int meshResourceId;    // index into MeshResourceData buffer
    public int materialId;
    public int proxyId;           // reserved
    public int visible;           // reserved
};

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
    public MeshInstanceData GetMeshData(uint instanceId);
}

StructuredBuffer<float4x4> u_vBonesMatrix;
StructuredBuffer<int> u_vBonesOffset;
