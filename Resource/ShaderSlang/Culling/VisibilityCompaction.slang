import InternalModule;
import Model.ModelModule;
import Culling.CullingModule;

// ==================== Visibility Compaction Utilities ====================
// This shader is used for further optimization, merging multiple small clusters into larger draw calls

public struct CompactedDrawCommand
{
    uint batchStartIndex;
    uint batchClusterCount;
    uint meshInstanceIndex;
};

// ==================== Compute Shader: Visibility Buffer Compaction ====================
// Optional optimization stage: sort and compact candidate clusters

public struct NaniteVisibilityCompactionParams
{ 
    StructuredBuffer<CandidateCluster> candidateClusters;
    StructuredBuffer<uint> candidateCount;

    RWStructuredBuffer<CandidateCluster> outSortedCandidates;
};


[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteVisibilityCompactionCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,
    ParameterBlock<NaniteVisibilityCompactionParams> input
   
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = input.candidateCount[0];

    if (candidateIndex >= totalCandidates)
        return;

    // Simple copy (in practice, distance-based sorting can be implemented here)
    // Sorting can improve cache coherence and reduce state switches
    input.outSortedCandidates[candidateIndex] = input.candidateClusters[candidateIndex];
}

// ==================== Compute Shader: Batch Draw Commands ====================
// Advanced optimization: try to batch draw consecutive clusters of the same MeshInstance

public struct NaniteBatchDrawCommandsParams
{
    StructuredBuffer<CandidateCluster> sortedCandidates;
    StructuredBuffer<uint> candidateCount;
    StructuredBuffer<ClusterData> clusters;

    RWStructuredBuffer<CompactedDrawCommand> outCompactedCommands;
    RWStructuredBuffer<uint> outCompactedCount;
};

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteBatchDrawCommandsCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    ParameterBlock<NaniteBatchDrawCommandsParams> input
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = input.candidateCount[0];

    if (candidateIndex >= totalCandidates)
        return;

    CandidateCluster currentCandidate = input.sortedCandidates[candidateIndex];

    // Check if can be merged with the previous candidate
    bool canBatch = false;
    if (candidateIndex > 0)
    {
        CandidateCluster prevCandidate = input.sortedCandidates[candidateIndex - 1];

        // Can only merge if same MeshInstance and indices are consecutive
        ClusterData currentCluster = input.clusters[currentCandidate.clusterIndex];
        ClusterData prevCluster = input.clusters[prevCandidate.clusterIndex];

        canBatch = (currentCandidate.meshInstanceIndex == prevCandidate.meshInstanceIndex) &&
                   (currentCluster.indexOffset == prevCluster.indexOffset + prevCluster.indexCount);
    }

    if (!canBatch)
    {
        // Create new compacted command
        uint compactedIndex;
        InterlockedAdd(input.outCompactedCount[0], 1, compactedIndex);

        CompactedDrawCommand cmd;
        cmd.batchStartIndex = candidateIndex;
        cmd.batchClusterCount = 1;
        cmd.meshInstanceIndex = currentCandidate.meshInstanceIndex;

        input.outCompactedCommands[compactedIndex] = cmd;
    }
    // If can be merged, handled by subsequent scan stage
}

// ==================== Compute Shader: Occlusion Culling (Future) ====================
// Reserved interface: occlusion culling can be added in the future

public struct NaniteOcclusionCullingParams
{
    NaniteCullingParams naniteCullingParams;
    Texture2D<float> hierarchicalZBuffer;
    StructuredBuffer<CandidateCluster> candidateClusters;
    StructuredBuffer<ClusterData> clusters;
    StructuredBuffer<MeshInstanceRenderProxy> meshInstances;

    RWStructuredBuffer<uint> outVisibilityMask;
};

[shader("compute")]
[numthreads(8, 8, 1)]
public void NaniteOcclusionCullingCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    ParameterBlock<NaniteOcclusionCullingParams> input
)
{
    uint candidateIndex = dispatchThreadID.x;

    // Use naniteCullingParams.renderData from CullingModule
    NaniteRenderData renderData = input.naniteCullingParams.renderData;

    // TODO: Implement Hi-Z occlusion culling
    // 1. Project cluster bounding sphere to screen space
    // 2. Query Hierarchical Z-Buffer
    // 3. If occluded, clear the corresponding visibility bit
}
