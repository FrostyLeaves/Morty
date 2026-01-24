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

    // Frustum
    FrustumUtils.Frustum frustum;
};

// Candidate Cluster structure (output after LOD selection)
public struct CandidateCluster
{
    public uint clusterIndex;
    public uint groupIndex;
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
        if (group.groupLinkCount == 0)
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
        FrustumUtils.Frustum frustum)
    {
        for (int i = 0; i < 6; ++i)
        {
            float distance = dot(frustum.planes[i].xyz, center) + frustum.planes[i].w;
            if (distance > radius)
                return false;
        }
        return true;
    }

    // Perform culling test on Cluster
    public static bool CullCluster(
        ClusterData cluster,
        float4x4 worldMatrix,
        FrustumUtils.Frustum frustum)
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
        return SphereInFrustum(worldCenter, worldRadius, frustum);
    }

    // Perform culling test on ClusterGroup
    public static bool CullClusterGroup(
        ClusterGroupData group,
        float4x4 worldMatrix,
        FrustumUtils.Frustum frustum)
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
        return SphereInFrustum(worldCenter, worldRadius, frustum);
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

    // BFS traverse ClusterGroup DAG and select appropriate LOD
    public static void TraverseLODTree(
        uint meshInstanceIndex,
        MeshInstanceRenderProxy meshInstance,
        MeshResourceData meshResource,  // Mesh resource data (contains ClusterGroup root node info)
        StructuredBuffer<ClusterGroupData> clusterGroups,
        StructuredBuffer<ClusterData> clusters,
        StructuredBuffer<int> groupLinks,  // DAG child group indices
        NaniteRenderData renderData,
        RWStructuredBuffer<CandidateCluster> outCandidateClusters,
        RWStructuredBuffer<uint> outCandidateCount)
    {
        // Initialize traversal stack
        StackNode stack[NANITE_MAX_TRAVERSAL_DEPTH];
        uint stackSize = 0;

        for (int linkIdx = 0; linkIdx < meshResource.rootClusterGroupCount; ++linkIdx)
        {
            int globalLinkIdx = meshResource.rootClusterGroupIndex + linkIdx;
            // groupLinks stores local child group IDs, convert to global by adding clusterGroupBeginIndex
            int localChildId = groupLinks[globalLinkIdx];

            uint rootGroupIndex = meshResource.rootClusterGroupIndex + localChildId;

            // First check if root group is visible
            if (NaniteCulling.CullClusterGroup(
                clusterGroups[rootGroupIndex],
                meshInstance.matWorld,
                renderData.frustum))
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

            shouldSubdivide = currentNode.depth < 2;

            if (shouldSubdivide && currentNode.depth < NANITE_MAX_TRAVERSAL_DEPTH - 1)
            {
                // Traverse child ClusterGroups via DAG links
                for (uint i = 0; i < currentGroup.groupLinkCount; ++i)
                {
                    int globallinkIdx = currentGroup.groupLinkOffset + i;
                    // groupLinks stores local child group IDs, convert to global by adding clusterGroupBeginIndex
                    int localChildId = groupLinks[globallinkIdx];
                    uint childGroupIndex = meshResource.clusterGroupBeginIndex + localChildId;

                    // Frustum culling
                    if (NaniteCulling.CullClusterGroup(
                        clusterGroups[childGroupIndex],
                        meshInstance.matWorld,
                        renderData.frustum))
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

                    // Frustum culling
                    if (NaniteCulling.CullCluster(
                        cluster,
                        meshInstance.matWorld,
                        renderData.frustum))
                    {
                        // Output candidate cluster
                        uint candidateIndex;
                        InterlockedAdd(outCandidateCount[0], 1, candidateIndex);

                        CandidateCluster candidate;
                        candidate.clusterIndex = clusterIndex;
                        candidate.meshInstanceIndex = meshInstanceIndex;
                        candidate.groupIndex = currentNode.groupIndex;

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
