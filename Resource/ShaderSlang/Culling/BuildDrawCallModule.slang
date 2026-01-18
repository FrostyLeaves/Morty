module BuildDrawCallModule;

import InternalModule;
import Model.ModelModule;
import Culling.CullingModule;

static const uint MAX_DRAW_CALLS_PER_GROUP = 1024;

public struct BuildDrawCallFillParams
{
    BuildDrawCallParams buildDrawCallParams;
    StructuredBuffer<CandidateCluster> candidateClusters;
    StructuredBuffer<uint> candidateCount;
    StructuredBuffer<MeshInstanceRenderProxy> meshInstances;
    StructuredBuffer<ClusterData> clusters;

    RWStructuredBuffer<IndexedIndirectCommand> outDrawIndirect;
    RWStructuredBuffer<uint> outDrawGroupCount;
};

[[vk::binding(0, 0)]] ParameterBlock<BuildDrawCallFillParams> buildDrawCallInput;

[shader("compute")]
[numthreads(64, 1, 1)]
public void BuildDrawCallFillCS(
    uint3 dispatchThreadID: SV_DispatchThreadID
)
{
    BuildDrawCallFillParams input = buildDrawCallInput;

    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = input.candidateCount[0];
    if (candidateIndex >= totalCandidates)
        return;

    CandidateCluster candidate = input.candidateClusters[candidateIndex];
    // batchGroupId is treated as the batch group index.
    uint groupId = (uint)input.meshInstances[candidate.meshInstanceIndex].batchGroupId;
    if (groupId >= input.buildDrawCallParams.drawGroupCount)
        return;

    uint localIndex;
    InterlockedAdd(input.outDrawGroupCount[groupId], 1, localIndex);
    if (localIndex >= MAX_DRAW_CALLS_PER_GROUP)
        return;

    uint writeIndex = groupId * MAX_DRAW_CALLS_PER_GROUP + localIndex;

    ClusterData cluster = input.clusters[candidate.clusterIndex];

    IndexedIndirectCommand cmd;
    cmd.indexCount = cluster.indexCount;
    cmd.instanceCount = 1;
    cmd.firstIndex = cluster.indexOffset;
    cmd.vertexOffset = 0;
    cmd.firstInstance = candidate.meshInstanceIndex;

    input.outDrawIndirect[writeIndex] = cmd;
}
