module BuildDrawCallModule;

import InternalModule;
import Model.ModelModule;
import Culling.CullingModule;

static const uint MAX_DRAW_CALLS_PER_GROUP = 1024;

[shader("compute")]
[numthreads(64, 1, 1)]
public void BuildDrawCallFillCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // Input
    StructuredBuffer<CandidateCluster> candidateClusters,
    StructuredBuffer<uint> candidateCount,
    StructuredBuffer<MeshInstanceData> meshInstances,
    StructuredBuffer<ClusterData> clusters,

    // Output
    RWStructuredBuffer<IndexedIndirectCommand> outDrawIndirect,
    RWStructuredBuffer<uint> outDrawGroupCount
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = candidateCount[0];
    if (candidateIndex >= totalCandidates)
        return;

    CandidateCluster candidate = candidateClusters[candidateIndex];
    // batchGroupId is treated as the batch group index.
    uint groupId = (uint)meshInstances[candidate.meshInstanceIndex].batchGroupId;
    if (groupId >= buildDrawCallParams.drawGroupCount)
        return;

    uint localIndex;
    InterlockedAdd(outDrawGroupCount[groupId], 1, localIndex);
    if (localIndex >= MAX_DRAW_CALLS_PER_GROUP)
        return;

    uint writeIndex = groupId * MAX_DRAW_CALLS_PER_GROUP + localIndex;

    ClusterData cluster = clusters[candidate.clusterIndex];

    IndexedIndirectCommand cmd;
    cmd.indexCount = cluster.indexCount;
    cmd.instanceCount = 1;
    cmd.firstIndex = cluster.indexOffset;
    cmd.vertexOffset = 0;
    cmd.firstInstance = candidate.meshInstanceIndex;

    outDrawIndirect[writeIndex] = cmd;
}
