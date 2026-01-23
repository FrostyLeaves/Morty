implementing InternalModule;

[StructLayout("MClusterRenderData")]
public struct ClusterData
{
    public float3 position;
    public float radius;
    
    public float error;
    public uint indexOffset;
    public uint indexCount;
    public uint valid;
};

[StructLayout("MClusterGroupRenderData")]
public struct ClusterGroupData
{
    public float3 position;
    public float radius;

    public float error;
    public uint valid;
    public uint groupLinkOffset;
    public uint groupLinkCount;

    public uint clusterBeginIndex;
    public uint clusterCount;
    public uint vertexBeginIndex;
    public uint indexBeginIndex;
};

// Mesh resource level data, used for finding ClusterGroup root nodes during GPU culling
[StructLayout("MMeshResourceRenderData")]
public struct MeshResourceData
{
    public int clusterBeginIndex      ;
    public int clusterGroupBeginIndex ;
    public int rootClusterGroupIndex  ;
    public int rootClusterGroupCount  ;
};

[StructLayout("MMeshInstanceRenderProxy")]
public struct MeshInstanceRenderProxy
{
    public float4x4 matWorld;

    public int meshResourceId;    // index into MeshResourceData buffer
    public int batchGroupId;      // index into MaterialBatchGroup buffer
    public int materialInstanceId;
    public int proxyId;           // reserved

    public int visible;           // reserved
};

public struct RenderSceneParameters
{
    public StructuredBuffer<ClusterData> clusters;
    public StructuredBuffer<ClusterGroupData> clusterGroups;
    public StructuredBuffer<MeshResourceData> meshResources;
    public StructuredBuffer<MeshInstanceRenderProxy> instanceProxy;
};
