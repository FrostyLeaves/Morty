module MeshInstanceCullingModule;

import Culling.CullingModule;
import InternalModule;

// ==================== Compute Shader: Nanite Instance Culling ====================
// Phase 1: Iterate all MeshInstances, perform LOD selection and frustum culling

public struct NaniteInstanceCullingParams
{
    NaniteCullingParams naniteCullingParams;
    StructuredBuffer<MeshInstanceRenderProxy> meshInstances;
    StructuredBuffer<MeshResourceData> meshResources;
    StructuredBuffer<ClusterGroupData> clusterGroups;
    StructuredBuffer<ClusterData> clusters;
    StructuredBuffer<int> groupLinks;  // DAG child group indices

    RWStructuredBuffer<CandidateCluster> outCandidateClusters;
    RWStructuredBuffer<uint> outCandidateCount;
};
[[vk::binding(0, 0)]] ParameterBlock<NaniteInstanceCullingParams> instanceCullingInput;

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteInstanceCullingCS(
    uint3 dispatchThreadID: SV_DispatchThreadID,
)
{
    NaniteInstanceCullingParams input = instanceCullingInput;

    uint instanceIndex = dispatchThreadID.x;
    if (instanceIndex >= input.naniteCullingParams.instanceCount)
        return;

    MeshInstanceRenderProxy meshInstance = input.meshInstances[instanceIndex];

    // Look up MeshResourceData indirectly via meshResourceId
    MeshResourceData meshResource = input.meshResources[meshInstance.meshResourceId];

    // Traverse LOD DAG and select appropriate clusters
    NaniteTraversal.TraverseLODTree(
        instanceIndex,
        meshInstance,
        meshResource,  // Pass MeshResourceData
        input.clusterGroups,
        input.clusters,
        input.groupLinks,  // DAG child group indices
        input.naniteCullingParams.renderData,
        input.outCandidateClusters,
        input.outCandidateCount
    );
}
