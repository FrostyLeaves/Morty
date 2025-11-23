/**
 * @file ClusterCulling.slang
 * @brief GPU-based cluster culling compute shader
 *
 * This shader performs frustum culling on mesh clusters and generates
 * indirect draw commands for visible clusters.
 */

module ClusterCullingModule;

__include InternalModule;

/**
 * @brief Cluster bounding sphere data
 */
public struct ClusterBounds
{
    public float3 center;      // World space center
    public float radius;       // Bounding sphere radius
    public uint indexOffset;   // Offset in global index buffer
    public uint indexCount;    // Number of indices (usually 192 for 64 triangles)
    public uint instanceId;    // Instance ID for this cluster
    public uint _padding;      // Padding for alignment
};

/**
 * @brief Frustum culling parameters
 */
public struct CullingParams
{
    public float4 frustumPlanes[6];  // View frustum planes (left, right, top, bottom, near, far)
    public uint clusterCount;        // Total number of clusters to test
    public uint _padding[3];         // Padding for alignment
};

/**
 * @brief Test if a sphere is inside the frustum
 * @param center Sphere center in world space
 * @param radius Sphere radius
 * @param frustumPlanes Array of 6 frustum planes
 * @return true if sphere is inside or intersecting frustum
 */
public bool FrustumCullSphere(float3 center, float radius, float4 frustumPlanes[6])
{
    // Test against each frustum plane
    for (uint i = 0; i < 6; ++i)
    {
        float4 plane = frustumPlanes[i];
        float distance = dot(plane.xyz, center) + plane.w;

        // If sphere is completely outside any plane, it's culled
        if (distance < -radius)
            return false;
    }

    return true;
}

/**
 * @brief Cluster culling compute shader entry point
 *
 * Each thread processes one cluster:
 * 1. Load cluster bounds
 * 2. Perform frustum culling test
 * 3. If visible, append indirect draw command to output buffer
 */
[shader("compute")]
[numthreads(64, 1, 1)]
public void ClusterCullingCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    // Input buffers
    StructuredBuffer<ClusterBounds> clusterBounds,
    ParameterBlock<CullingParams> cullingParams,

    // Output buffers
    RWStructuredBuffer<IndexedIndirectCommand> indirectCommands,
    RWByteAddressBuffer commandCounterBuffer  // Single uint for atomic counter
)
{
    uint clusterIdx = dispatchThreadID.x;

    // Early exit if thread ID exceeds cluster count
    if (clusterIdx >= cullingParams.clusterCount)
        return;

    // Load cluster data
    ClusterBounds cluster = clusterBounds[clusterIdx];

    // Perform frustum culling
    bool isVisible = FrustumCullSphere(
        cluster.center,
        cluster.radius,
        cullingParams.frustumPlanes
    );

    if (isVisible)
    {
        // Atomically increment command counter and get write index
        uint writeIndex;
        commandCounterBuffer.InterlockedAdd(0, 1, writeIndex);

        // Write indirect draw command
        IndexedIndirectCommand cmd;
        cmd.indexCount = cluster.indexCount;
        cmd.instanceCount = 1;
        cmd.firstIndex = cluster.indexOffset;
        cmd.vertexOffset = 0;
        cmd.firstInstance = cluster.instanceId;

        indirectCommands[writeIndex] = cmd;
    }
}

/**
 * @brief Simple version without atomic operations (for debugging)
 * Assumes all clusters are visible and writes sequential commands
 */
[shader("compute")]
[numthreads(64, 1, 1)]
public void ClusterCullingSimpleCS(
    uint3 dispatchThreadID : SV_DispatchThreadID,

    StructuredBuffer<ClusterBounds> clusterBounds,
    ParameterBlock<CullingParams> cullingParams,
    RWStructuredBuffer<IndexedIndirectCommand> indirectCommands
)
{
    uint clusterIdx = dispatchThreadID.x;

    if (clusterIdx >= cullingParams.clusterCount)
        return;

    ClusterBounds cluster = clusterBounds[clusterIdx];

    // Write command without culling (for testing)
    IndexedIndirectCommand cmd;
    cmd.indexCount = cluster.indexCount;
    cmd.instanceCount = 1;
    cmd.firstIndex = cluster.indexOffset;
    cmd.vertexOffset = 0;
    cmd.firstInstance = cluster.instanceId;

    indirectCommands[clusterIdx] = cmd;
}
