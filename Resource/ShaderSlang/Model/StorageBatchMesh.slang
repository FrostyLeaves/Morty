implementing ModelModule;
__include "CommonModel";

public struct StorageBatchMeshData : IBatchMeshData
{
    public MeshInstanceRenderProxy GetInstanceProxy(uint instanceId)
    {
        return GlobalData.scene.instanceProxy[instanceId];
    }
}
