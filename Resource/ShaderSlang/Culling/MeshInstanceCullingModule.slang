module MeshInstanceCullingModule;

import InternalModule;
import Model.ModelModule;
import Culling.CullingModule;

// ==================== Compute Shader: Nanite Instance Culling ====================
// Phase 1: Iterate all MeshInstances, perform LOD selection and frustum culling

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteInstanceCullingCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // Input
    StructuredBuffer<MeshInstanceData> meshInstances,
    StructuredBuffer<MeshResourceData> meshResources,  // Mesh resource data buffer
    StructuredBuffer<ClusterGroupData> clusterGroups,
    StructuredBuffer<ClusterData> clusters,

    // Output
    RWStructuredBuffer<CandidateCluster> outCandidateClusters,
    RWStructuredBuffer<uint> outCandidateCount  // [0] = count
)
{
    uint instanceIndex = dispatchThreadID.x;
    if (instanceIndex >= naniteCullingParams.instanceCount)
        return;

    MeshInstanceData meshInstance = meshInstances[instanceIndex];

    // Look up MeshResourceData indirectly via meshResourceId
    MeshResourceData meshResource = meshResources[meshInstance.meshResourceId];

    // Traverse LOD tree and select appropriate clusters
    NaniteTraversal.TraverseLODTree(
        instanceIndex,
        meshInstance,
        meshResource,  // Pass MeshResourceData
        clusterGroups,
        clusters,
        naniteCullingParams.renderData,
        outCandidateClusters,
        outCandidateCount
    );
}

// ==================== Compute Shader: Generate Indirect Draw Commands ====================
// Phase 2: Convert candidate Clusters to DrawIndexedIndirect commands

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteGenerateIndirectDrawCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // Input
    StructuredBuffer<CandidateCluster> candidateClusters,
    StructuredBuffer<uint> candidateCount,
    StructuredBuffer<ClusterData> clusters,

    // Output
    RWStructuredBuffer<IndexedIndirectCommand> outIndirectCommands,
    RWStructuredBuffer<uint> outInstanceIndices  // Used to look up corresponding MeshInstance during draw
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = candidateCount[0];

    if (candidateIndex >= totalCandidates)
        return;

    CandidateCluster candidate = candidateClusters[candidateIndex];
    ClusterData cluster = clusters[candidate.clusterIndex];

    // Build DrawIndexedIndirect command
    IndexedIndirectCommand cmd;
    cmd.indexCount = cluster.indexCount;
    cmd.instanceCount = 1;
    cmd.firstIndex = cluster.indexOffset;
    cmd.vertexOffset = 0;
    cmd.firstInstance = candidateIndex;  // Use candidate index as instance ID

    outIndirectCommands[candidateIndex] = cmd;
    outInstanceIndices[candidateIndex] = candidate.meshInstanceIndex;
}

// ==================== Compute Shader: Initialize Buffers ====================
// Initialization phase: Clear counters

[shader("compute")]
[numthreads(1, 1, 1)]
public void NaniteInitBuffersCS(
    RWStructuredBuffer<uint> outCandidateCount
)
{
    outCandidateCount[0] = 0;
}

// ==================== Compute Shader: Prepare Indirect Dispatch ====================
// Prepare indirect dispatch: Calculate required thread group count

[shader("compute")]
[numthreads(1, 1, 1)]
public void NanitePrepareIndirectDispatchCS(
    StructuredBuffer<uint> candidateCount,
    RWStructuredBuffer<uint3> outDispatchArgs  // [0].xyz = (groupCountX, groupCountY, groupCountZ)
)
{
    uint totalCandidates = candidateCount[0];
    uint groupCount = (totalCandidates + 63) / 64;  // 64 threads per group

    outDispatchArgs[0] = uint3(groupCount, 1, 1);
}
