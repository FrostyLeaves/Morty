implementing CullingModule;

// ==================== Nanite Constants and Configuration ====================

public static const uint NANITE_MAX_TRAVERSAL_DEPTH = 16;
public static const float NANITE_ERROR_THRESHOLD = 1.0; // Screen-space error threshold (pixels)

// ==================== Nanite Utility Structures ====================

public struct NaniteRenderData
{
    float4x4 viewProjMatrix;
    float4x4 viewMatrix;
    float3 cameraPositionWS;
    float2 screenSize;
    float nearClip;
    float farClip;

    // Frustum planes
    FrustumUtils.Plane frustumPlanes[6];
};

// Candidate Cluster structure (output after LOD selection)
public struct CandidateCluster
{
    public uint clusterIndex;
    public uint meshInstanceIndex;
    public float distanceToCamera;
};

// ==================== Nanite LOD Selection Utilities ====================

public struct NaniteLODSelector
{
    // Calculate screen-space projection error
    public static float CalculateScreenSpaceError(
        float3 worldPosition,
        float worldSpaceError,
        float distanceToCamera,
        float2 screenSize,
        float4x4 viewProjMatrix)
    {
        // Calculate projection error
        float4 projPos = mul(viewProjMatrix, float4(worldPosition, 1.0));

        // Perspective division
        if (projPos.w > 0.0001)
        {
            float screenRadius = (worldSpaceError / projPos.w) * screenSize.y * 0.5;
            return screenRadius;
        }

        return 10000.0; // Return large value if behind camera
    }

    // Determine if subdivision to child nodes is needed
    public static bool ShouldSubdivide(
        ClusterGroupData group,
        float3 cameraPos,
        float2 screenSize,
        float4x4 viewProjMatrix,
        float errorThreshold)
    {
        // No subdivision needed if no child nodes
        if (group.childGroupCount == 0)
            return false;

        // Calculate distance to camera
        float distanceToCamera = length(group.position - cameraPos);

        // Calculate screen-space error
        float screenError = CalculateScreenSpaceError(
            group.position,
            group.error,
            distanceToCamera,
            screenSize,
            viewProjMatrix
        );

        // Subdivide if error exceeds threshold
        return screenError > errorThreshold;
    }
};

// ==================== Nanite Frustum Culling Utilities ====================

public struct NaniteCulling
{
    // Sphere-frustum culling test
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

    // Perform culling test on Cluster
    public static bool CullCluster(
        ClusterData cluster,
        float4x4 worldMatrix,
        FrustumUtils.Plane frustumPlanes[6])
    {
        // Transform cluster bounding sphere to world space
        float3 worldCenter = mul(worldMatrix, float4(cluster.position, 1.0)).xyz;

        // Get scale factor (for radius adjustment)
        float3 scaleX = float3(worldMatrix[0][0], worldMatrix[1][0], worldMatrix[2][0]);
        float3 scaleY = float3(worldMatrix[0][1], worldMatrix[1][1], worldMatrix[2][1]);
        float3 scaleZ = float3(worldMatrix[0][2], worldMatrix[1][2], worldMatrix[2][2]);
        float maxScale = max(max(length(scaleX), length(scaleY)), length(scaleZ));
        float worldRadius = cluster.radius * maxScale;

        // Frustum culling
        return SphereInFrustum(worldCenter, worldRadius, frustumPlanes);
    }

    // Perform culling test on ClusterGroup
    public static bool CullClusterGroup(
        ClusterGroupData group,
        float4x4 worldMatrix,
        FrustumUtils.Plane frustumPlanes[6])
    {
        // Transform group bounding sphere to world space
        float3 worldCenter = mul(worldMatrix, float4(group.position, 1.0)).xyz;

        // Get scale factor
        float3 scaleX = float3(worldMatrix[0][0], worldMatrix[1][0], worldMatrix[2][0]);
        float3 scaleY = float3(worldMatrix[0][1], worldMatrix[1][1], worldMatrix[2][1]);
        float3 scaleZ = float3(worldMatrix[0][2], worldMatrix[1][2], worldMatrix[2][2]);
        float maxScale = max(max(length(scaleX), length(scaleY)), length(scaleZ));
        float worldRadius = group.radius * maxScale;

        // Frustum culling
        return SphereInFrustum(worldCenter, worldRadius, frustumPlanes);
    }
};

// ==================== Nanite Traversal Utilities ====================

public struct NaniteTraversal
{
    // Traversal stack node
    public struct StackNode
    {
        uint groupIndex;
        uint depth;
    };

    // BFS traverse ClusterGroup tree and select appropriate LOD
    public static void TraverseLODTree(
        uint meshInstanceIndex,
        MeshInstanceData meshInstance,
        MeshResourceData meshResource,  // Mesh resource data (contains ClusterGroup root node info)
        StructuredBuffer<ClusterGroupData> clusterGroups,
        StructuredBuffer<ClusterData> clusters,
        NaniteRenderData renderData,
        RWStructuredBuffer<CandidateCluster> outCandidateClusters,
        RWStructuredBuffer<uint> outCandidateCount)
    {
        // Initialize traversal stack
        StackNode stack[NANITE_MAX_TRAVERSAL_DEPTH];
        uint stackSize = 0;

        // Push all root ClusterGroups to stack (indirect lookup via MeshResourceData)
        for (int i = 0; i < meshResource.rootClusterGroupCount; ++i)
        {
            uint rootGroupIndex = meshResource.rootClusterGroupBeginIndex + i;

            // First check if root group is visible
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

        // BFS traversal
        while (stackSize > 0)
        {
            // Pop top node from stack
            stackSize--;
            StackNode currentNode = stack[stackSize];
            ClusterGroupData currentGroup = clusterGroups[currentNode.groupIndex];

            // Determine if subdivision to child nodes is needed
            bool shouldSubdivide = NaniteLODSelector.ShouldSubdivide(
                currentGroup,
                renderData.cameraPositionWS,
                renderData.screenSize,
                renderData.viewProjMatrix,
                NANITE_ERROR_THRESHOLD
            );

            if (shouldSubdivide && currentNode.depth < NANITE_MAX_TRAVERSAL_DEPTH - 1)
            {
                // Traverse child ClusterGroups
                for (uint i = 0; i < currentGroup.childGroupCount; ++i)
                {
                    uint childGroupIndex = currentGroup.firstGroupId + i;

                    // Frustum culling
                    if (NaniteCulling.CullClusterGroup(
                        clusterGroups[childGroupIndex],
                        meshInstance.matWorld,
                        renderData.frustumPlanes))
                    {
                        // Push to stack
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
                // No subdivision needed, output all clusters of current group
                for (uint i = 0; i < currentGroup.clusterCount; ++i)
                {
                    uint clusterIndex = currentGroup.clusterBeginIndex + i;
                    ClusterData cluster = clusters[clusterIndex];

                    // Skip invalid clusters
                    if (cluster.valid == 0)
                        continue;

                    // Frustum culling
                    if (NaniteCulling.CullCluster(
                        cluster,
                        meshInstance.matWorld,
                        renderData.frustumPlanes))
                    {
                        // Output candidate cluster
                        uint candidateIndex;
                        InterlockedAdd(outCandidateCount[0], 1, candidateIndex);

                        CandidateCluster candidate;
                        candidate.clusterIndex = clusterIndex;
                        candidate.meshInstanceIndex = meshInstanceIndex;

                        // Calculate distance to camera (for later sorting)
                        float3 worldPos = mul(meshInstance.matWorld, float4(cluster.position, 1.0)).xyz;
                        candidate.distanceToCamera = length(worldPos - renderData.cameraPositionWS);

                        outCandidateClusters[candidateIndex] = candidate;
                    }
                }
            }
        }
    }
};
