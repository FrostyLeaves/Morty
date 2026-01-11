implementing ModelModule;

#include "Internal/PreDefine.slang"

public struct ClusterData
{
    public uint indexOffset;
    public uint indexCount;
    public uint valid;

    public float3 position;
    public float radius;
    public float error;
};

public struct ClusterGroupData
{
    public uint valid;
    public uint parentGroupId;
    public uint firstGroupId;
    public uint childGroupCount;

    public uint clusterBeginIndex;
    public uint clusterCount;

    public float3 position;
    public float radius;
    public float error;
};

// Mesh resource level data, used for finding ClusterGroup root nodes during GPU culling
public struct MeshResourceData
{
    public int rootClusterGroupBeginIndex;
    public int rootClusterGroupCount;
};

public struct MeshInstanceData
{
    public float4x4 matWorld;
    public int meshResourceId;    // index into MeshResourceData buffer
    public int batchGroupId;      // index into MaterialBatchGroup buffer
    public int materialInstanceId;
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
