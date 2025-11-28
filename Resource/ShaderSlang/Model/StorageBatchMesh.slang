implementing ModelModule;
__include "CommonModel";

internal struct InstanceGroupData
{
    int instanceBeginIndex;
};

[MeshInstance] StructuredBuffer<MeshData> meshData;
ParameterBlock<InstanceGroupData> instanceGroupData;

struct StorageBatchMeshData : IBatchMeshData
{

    MeshData GetMeshData(uint instanceId)
    {
        return meshData[instanceId - instanceGroupData.instanceBeginIndex];
    }
}
