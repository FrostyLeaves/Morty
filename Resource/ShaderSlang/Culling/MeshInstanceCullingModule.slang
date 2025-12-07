implementing InternalModule;

__include "NaniteCulling";

// ==================== Compute Shader: Nanite Instance Culling ====================
// 第一阶段：遍历所有 MeshInstance，进行 LOD 选择和视锥体剔除

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteInstanceCullingCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // 输入
    uniform NaniteRenderData renderData,
    uniform uint instanceCount,
    StructuredBuffer<MeshInstanceData> meshInstances,
    StructuredBuffer<ClusterGroupData> clusterGroups,
    StructuredBuffer<ClusterData> clusters,

    // 输出
    RWStructuredBuffer<CandidateCluster> outCandidateClusters,
    RWStructuredBuffer<uint> outCandidateCount  // [0] = count
)
{
    uint instanceIndex = dispatchThreadID.x;
    if (instanceIndex >= instanceCount)
        return;

    MeshInstanceData meshInstance = meshInstances[instanceIndex];

    // 遍历 LOD 树，选择合适的 cluster
    NaniteTraversal.TraverseLODTree(
        instanceIndex,
        meshInstance,
        clusterGroups,
        clusters,
        renderData,
        outCandidateClusters,
        outCandidateCount
    );
}

// ==================== Compute Shader: Generate Indirect Draw Commands ====================
// 第二阶段：将候选 Cluster 转换为 DrawIndexedIndirect 命令

[shader("compute")]
[numthreads(64, 1, 1)]
public void NaniteGenerateIndirectDrawCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // 输入
    StructuredBuffer<CandidateCluster> candidateClusters,
    StructuredBuffer<uint> candidateCount,
    StructuredBuffer<ClusterData> clusters,

    // 输出
    RWStructuredBuffer<IndexedIndirectCommand> outIndirectCommands,
    RWStructuredBuffer<uint> outInstanceIndices  // 用于在绘制时查找对应的 MeshInstance
)
{
    uint candidateIndex = dispatchThreadID.x;
    uint totalCandidates = candidateCount[0];

    if (candidateIndex >= totalCandidates)
        return;

    CandidateCluster candidate = candidateClusters[candidateIndex];
    ClusterData cluster = clusters[candidate.clusterIndex];

    // 构建 DrawIndexedIndirect 命令
    IndexedIndirectCommand cmd;
    cmd.indexCount = cluster.indexCount;
    cmd.instanceCount = 1;
    cmd.firstIndex = cluster.indexOffset;
    cmd.vertexOffset = 0;
    cmd.firstInstance = candidateIndex;  // 使用候选索引作为 instance ID

    outIndirectCommands[candidateIndex] = cmd;
    outInstanceIndices[candidateIndex] = candidate.meshInstanceIndex;
}

// ==================== Compute Shader: Initialize Buffers ====================
// 初始化阶段：清空计数器

[shader("compute")]
[numthreads(1, 1, 1)]
public void NaniteInitBuffersCS(
    RWStructuredBuffer<uint> outCandidateCount
)
{
    outCandidateCount[0] = 0;
}

// ==================== Compute Shader: Prepare Indirect Dispatch ====================
// 准备间接派发：计算需要的线程组数量

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
