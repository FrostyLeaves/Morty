implementing InternalModule;

__include "NaniteCulling";

// ==================== 可见性压缩工具 ====================
// 这个着色器用于进一步优化，将多个小的 cluster 合并成更大的 draw call

public struct CompactedDrawCommand
{
    uint batchStartIndex;
    uint batchClusterCount;
    uint meshInstanceIndex;
};

// ==================== Compute Shader: Visibility Buffer Compaction ====================
// 可选优化阶段：对候选 Cluster 进行排序和压缩

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteVisibilityCompactionCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID,

    // 输入
    StructuredBuffer<CandidateCluster> candidateClusters,
    StructuredBuffer<uint> candidateCount,

    // 输出（排序后的候选列表）
    RWStructuredBuffer<CandidateCluster> outSortedCandidates
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = candidateCount[0];

    if (candidateIndex >= totalCandidates)
        return;

    // 简单的拷贝（实际应用中这里可以实现基于距离的排序）
    // 排序可以提高缓存一致性，减少状态切换
    outSortedCandidates[candidateIndex] = candidateClusters[candidateIndex];
}

// ==================== Compute Shader: Batch Draw Commands ====================
// 高级优化：尝试批量绘制同一 MeshInstance 的连续 Cluster

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteBatchDrawCommandsCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // 输入
    StructuredBuffer<CandidateCluster> sortedCandidates,
    StructuredBuffer<uint> candidateCount,
    StructuredBuffer<ClusterData> clusters,

    // 输出
    RWStructuredBuffer<CompactedDrawCommand> outCompactedCommands,
    RWStructuredBuffer<uint> outCompactedCount
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = candidateCount[0];

    if (candidateIndex >= totalCandidates)
        return;

    CandidateCluster currentCandidate = sortedCandidates[candidateIndex];

    // 检查是否可以与前一个候选合并
    bool canBatch = false;
    if (candidateIndex > 0)
    {
        CandidateCluster prevCandidate = sortedCandidates[candidateIndex - 1];

        // 只有相同的 MeshInstance 且索引连续才能合并
        ClusterData currentCluster = clusters[currentCandidate.clusterIndex];
        ClusterData prevCluster = clusters[prevCandidate.clusterIndex];

        canBatch = (currentCandidate.meshInstanceIndex == prevCandidate.meshInstanceIndex) &&
                   (currentCluster.indexOffset == prevCluster.indexOffset + prevCluster.indexCount);
    }

    if (!canBatch)
    {
        // 创建新的压缩命令
        uint compactedIndex;
        InterlockedAdd(outCompactedCount[0], 1, compactedIndex);

        CompactedDrawCommand cmd;
        cmd.batchStartIndex = candidateIndex;
        cmd.batchClusterCount = 1;
        cmd.meshInstanceIndex = currentCandidate.meshInstanceIndex;

        outCompactedCommands[compactedIndex] = cmd;
    }
    // 如果可以合并，则由后续的扫描阶段处理
}

// ==================== Compute Shader: Occlusion Culling (Future) ====================
// 预留接口：未来可以添加遮挡剔除

[shader("compute")]
[numthreads(8, 8, 1)]
public void NaniteOcclusionCullingCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // 输入
    Texture2D<float> hierarchicalZBuffer,
    StructuredBuffer<CandidateCluster> candidateClusters,
    StructuredBuffer<ClusterData> clusters,
    StructuredBuffer<MeshInstanceData> meshInstances,
    uniform NaniteRenderData renderData,

    // 输出
    RWStructuredBuffer<uint> outVisibilityMask  // 位掩码，标记可见的候选
)
{
    uint candidateIndex = dispatchThreadID.x;

    // TODO: 实现 Hi-Z 遮挡剔除
    // 1. 将 cluster 的包围球投影到屏幕空间
    // 2. 查询 Hierarchical Z-Buffer
    // 3. 如果被遮挡，清除对应的可见性位
}
