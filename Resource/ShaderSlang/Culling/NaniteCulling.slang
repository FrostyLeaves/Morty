implementing InternalModule;

__include "../Model/CommonModel";
__include "../Internal/CommonStruct";
__include "../Internal/Utils";

// ==================== Nanite 常量和配置 ====================

public static const uint NANITE_MAX_TRAVERSAL_DEPTH = 16;
public static const float NANITE_ERROR_THRESHOLD = 1.0; // 屏幕空间误差阈值（像素）

// ==================== Nanite 工具结构 ====================

public struct NaniteRenderData
{
    float4x4 viewProjMatrix;
    float4x4 viewMatrix;
    float3 cameraPositionWS;
    float2 screenSize;
    float nearClip;
    float farClip;

    // 视锥体平面
    FrustumUtils.Plane frustumPlanes[6];
};

// 候选 Cluster 结构（用于 LOD 选择后的输出）
public struct CandidateCluster
{
    uint clusterIndex;
    uint meshInstanceIndex;
    float distanceToCamera;
};

// ==================== Nanite LOD 选择工具 ====================

public struct NaniteLODSelector
{
    // 计算屏幕空间投影误差
    public static float CalculateScreenSpaceError(
        float3 worldPosition,
        float worldSpaceError,
        float distanceToCamera,
        float2 screenSize,
        float4x4 viewProjMatrix)
    {
        // 计算投影误差
        float4 projPos = mul(viewProjMatrix, float4(worldPosition, 1.0));

        // 透视除法
        if (projPos.w > 0.0001)
        {
            float screenRadius = (worldSpaceError / projPos.w) * screenSize.y * 0.5;
            return screenRadius;
        }

        return 10000.0; // 如果在相机后面，返回一个大值
    }

    // 判断是否需要细分到子节点
    public static bool ShouldSubdivide(
        ClusterGroupData group,
        float3 cameraPos,
        float2 screenSize,
        float4x4 viewProjMatrix,
        float errorThreshold)
    {
        // 如果没有子节点，不需要细分
        if (group.childGroupCount == 0)
            return false;

        // 计算到相机的距离
        float distanceToCamera = length(group.position - cameraPos);

        // 计算屏幕空间误差
        float screenError = CalculateScreenSpaceError(
            group.position,
            group.error,
            distanceToCamera,
            screenSize,
            viewProjMatrix
        );

        // 如果误差大于阈值，需要细分
        return screenError > errorThreshold;
    }
};

// ==================== Nanite 视锥体剔除工具 ====================

public struct NaniteCulling
{
    // 球体视锥体剔除
    public static bool SphereInFrustum(
        float3 center,
        float radius,
        FrustumUtils.Plane frustumPlanes[6])
    {
        for (int i = 0; i < 6; ++i)
        {
            float distance = dot(frustumPlanes[i].normal, center) + frustumPlanes[i].distance;
            if (distance < -radius)
                return false;
        }
        return true;
    }

    // 对 Cluster 进行剔除测试
    public static bool CullCluster(
        ClusterData cluster,
        float4x4 worldMatrix,
        FrustumUtils.Plane frustumPlanes[6])
    {
        // 将 cluster 的包围球变换到世界空间
        float3 worldCenter = mul(worldMatrix, float4(cluster.position, 1.0)).xyz;

        // 获取缩放因子（用于调整半径）
        float3 scaleX = float3(worldMatrix[0][0], worldMatrix[1][0], worldMatrix[2][0]);
        float3 scaleY = float3(worldMatrix[0][1], worldMatrix[1][1], worldMatrix[2][1]);
        float3 scaleZ = float3(worldMatrix[0][2], worldMatrix[1][2], worldMatrix[2][2]);
        float maxScale = max(max(length(scaleX), length(scaleY)), length(scaleZ));
        float worldRadius = cluster.radius * maxScale;

        // 视锥体剔除
        return SphereInFrustum(worldCenter, worldRadius, frustumPlanes);
    }

    // 对 ClusterGroup 进行剔除测试
    public static bool CullClusterGroup(
        ClusterGroupData group,
        float4x4 worldMatrix,
        FrustumUtils.Plane frustumPlanes[6])
    {
        // 将 group 的包围球变换到世界空间
        float3 worldCenter = mul(worldMatrix, float4(group.position, 1.0)).xyz;

        // 获取缩放因子
        float3 scaleX = float3(worldMatrix[0][0], worldMatrix[1][0], worldMatrix[2][0]);
        float3 scaleY = float3(worldMatrix[0][1], worldMatrix[1][1], worldMatrix[2][1]);
        float3 scaleZ = float3(worldMatrix[0][2], worldMatrix[1][2], worldMatrix[2][2]);
        float maxScale = max(max(length(scaleX), length(scaleY)), length(scaleZ));
        float worldRadius = group.radius * maxScale;

        // 视锥体剔除
        return SphereInFrustum(worldCenter, worldRadius, frustumPlanes);
    }
};

// ==================== Nanite 遍历工具 ====================

public struct NaniteTraversal
{
    // 遍历栈节点
    public struct StackNode
    {
        uint groupIndex;
        uint depth;
    };

    // BFS 遍历 ClusterGroup 树，选择合适的 LOD
    public static void TraverseLODTree(
        uint meshInstanceIndex,
        MeshInstanceData meshInstance,
        StructuredBuffer<ClusterGroupData> clusterGroups,
        StructuredBuffer<ClusterData> clusters,
        NaniteRenderData renderData,
        RWStructuredBuffer<CandidateCluster> outCandidateClusters,
        RWStructuredBuffer<uint> outCandidateCount)
    {
        // 初始化遍历栈
        StackNode stack[NANITE_MAX_TRAVERSAL_DEPTH];
        uint stackSize = 0;

        // 将所有 root ClusterGroup 压入栈
        for (int i = 0; i < meshInstance.rootClusterGroupCount; ++i)
        {
            uint rootGroupIndex = meshInstance.rootClusterGroupBeginIndex + i;

            // 首先检查 root group 是否可见
            if (NaniteCulling.CullClusterGroup(
                clusterGroups[rootGroupIndex],
                meshInstance.matWorld,
                renderData.frustumPlanes))
            {
                stack[stackSize].groupIndex = rootGroupIndex;
                stack[stackSize].depth = 0;
                stackSize++;
            }
        }

        // BFS 遍历
        while (stackSize > 0)
        {
            // 弹出栈顶节点
            stackSize--;
            StackNode currentNode = stack[stackSize];
            ClusterGroupData currentGroup = clusterGroups[currentNode.groupIndex];

            // 判断是否需要细分到子节点
            bool shouldSubdivide = NaniteLODSelector.ShouldSubdivide(
                currentGroup,
                renderData.cameraPositionWS,
                renderData.screenSize,
                renderData.viewProjMatrix,
                NANITE_ERROR_THRESHOLD
            );

            if (shouldSubdivide && currentNode.depth < NANITE_MAX_TRAVERSAL_DEPTH - 1)
            {
                // 遍历子 ClusterGroup
                for (uint i = 0; i < currentGroup.childGroupCount; ++i)
                {
                    uint childGroupIndex = currentGroup.firstGroupId + i;

                    // 视锥体剔除
                    if (NaniteCulling.CullClusterGroup(
                        clusterGroups[childGroupIndex],
                        meshInstance.matWorld,
                        renderData.frustumPlanes))
                    {
                        // 压入栈
                        if (stackSize < NANITE_MAX_TRAVERSAL_DEPTH)
                        {
                            stack[stackSize].groupIndex = childGroupIndex;
                            stack[stackSize].depth = currentNode.depth + 1;
                            stackSize++;
                        }
                    }
                }
            }
            else
            {
                // 不需要细分，输出当前 group 的所有 clusters
                for (uint i = 0; i < currentGroup.clusterCount; ++i)
                {
                    uint clusterIndex = currentGroup.clusterBeginIndex + i;
                    ClusterData cluster = clusters[clusterIndex];

                    // 跳过无效的 cluster
                    if (cluster.valid == 0)
                        continue;

                    // 视锥体剔除
                    if (NaniteCulling.CullCluster(
                        cluster,
                        meshInstance.matWorld,
                        renderData.frustumPlanes))
                    {
                        // 输出候选 cluster
                        uint candidateIndex;
                        InterlockedAdd(outCandidateCount[0], 1, candidateIndex);

                        CandidateCluster candidate;
                        candidate.clusterIndex = clusterIndex;
                        candidate.meshInstanceIndex = meshInstanceIndex;

                        // 计算到相机的距离（用于后续排序）
                        float3 worldPos = mul(meshInstance.matWorld, float4(cluster.position, 1.0)).xyz;
                        candidate.distanceToCamera = length(worldPos - renderData.cameraPositionWS);

                        outCandidateClusters[candidateIndex] = candidate;
                    }
                }
            }
        }
    }
};
