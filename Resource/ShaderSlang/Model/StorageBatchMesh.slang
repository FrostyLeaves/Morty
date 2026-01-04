implementing ModelModule;
__include "CommonModel";

internal struct InstanceGroupData
{
    int instanceBeginIndex;
};

StructuredBuffer<ClusterData> clusters;
StructuredBuffer<ClusterGroupData> clusterGroups;
StructuredBuffer<MeshResourceData> meshResources;

[MeshInstance] StructuredBuffer<MeshInstanceData> meshData;
ParameterBlock<InstanceGroupData> instanceGroupData;

struct StorageBatchMeshData : IBatchMeshData
{
    MeshInstanceData GetMeshData(uint instanceId)
    {
        return meshData[instanceId - instanceGroupData.instanceBeginIndex];
    }
}
