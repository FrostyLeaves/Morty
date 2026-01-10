#include "MClusterBuilder.h"

#include "meshoptimizer.h"
#include <unordered_map>

using namespace morty;


void MClusterBuilder::Generate(MIMesh* mesh)
{
    InputData input;
    input.vertexData           = reinterpret_cast<float*>(mesh->GetVertices());
    input.vertexNum            = mesh->GetVerticesNum();
    input.vertexStride         = mesh->GetVertexStructSize();
    input.indexData            = mesh->GetIndices();
    input.indexNum             = mesh->GetIndicesNum();
    input.attributeProtectMask = mesh->GetAttributeProtectMask();
    input.simplifyWeight       = mesh->GetSimplifyWeight();

    BuildCluster(input);

    mesh->GetClusters()       = m_allClusters;
    mesh->GetClusterGroup()   = m_allGroups;
    mesh->GetClusterLodData() = m_lods;
    mesh->GetClusterPages()   = m_clusterPages;
}

std::vector<std::vector<int>> MClusterBuilder::PartitionCluster(
        const InputData&                 input,
        const std::vector<MClusterData>& clusters,
        const std::vector<int>&          pending,
        const std::vector<unsigned int>& remap
)
{
    std::vector<unsigned int> cluster_indices;
    std::vector<unsigned int> cluster_counts(pending.size());

    // copy cluster index data into a flat array for partitioning
    size_t                    total_index_count = 0;
    for (size_t i = 0; i < pending.size(); ++i) total_index_count += clusters[pending[i]].indices.size();

    cluster_indices.reserve(total_index_count);

    for (size_t i = 0; i < pending.size(); ++i)
    {
        const auto& cluster = clusters[pending[i]];

        cluster_counts[i] = unsigned(cluster.indices.size());

        for (size_t j = 0; j < cluster.indices.size(); ++j) cluster_indices.push_back(remap[cluster.indices[j]]);
    }

    // partition clusters into groups; the output is a partition id per cluster
    std::vector<unsigned int> cluster_part(pending.size());
    size_t                    partition_count = meshopt_partitionClusters(
            cluster_part.data(),
            cluster_indices.data(),
            cluster_indices.size(),
            cluster_counts.data(),
            cluster_counts.size(),
            input.vertexData,
            remap.size(),
            input.vertexStride,
            PartitionSize
    );

    // preallocate partitions for worst case
    std::vector<std::vector<int>> partitions(partition_count);
    for (size_t i = 0; i < partition_count; ++i) partitions[i].reserve(PartitionSize + PartitionSize / 3);

    std::vector<unsigned int> partition_remap;

    if (PartitionSort)
    {
        // compute partition points for sorting; any representative point will do, we use last cluster center for simplicity
        std::vector<float> partition_point(partition_count * 3);
        for (size_t i = 0; i < pending.size(); ++i)
            memcpy(&partition_point[cluster_part[i] * 3], &clusters[pending[i]].bounds.position, sizeof(Vector3));

        // sort partitions spatially; the output is a remap table from old index (partition id) to new index
        partition_remap.resize(partition_count);
        meshopt_spatialSortRemap(partition_remap.data(), partition_point.data(), partition_count, sizeof(float) * 3);
    }

    // distribute clusters into partitions, applying spatial order if requested
    for (size_t i = 0; i < pending.size(); ++i)
        partitions[partition_remap.empty() ? cluster_part[i] : partition_remap[cluster_part[i]]].push_back(pending[i]);

    return partitions;
}

void MClusterBuilder::LockBoundary(
        std::vector<unsigned char>&          locks,
        const std::vector<std::vector<int>>& groups,
        const std::vector<MClusterData>&     clusters,
        const std::vector<unsigned int>&     remap
)
{
    // for each remapped vertex, use bit 7 as temporary storage to indicate that the vertex has been used by a different group previously
    for (size_t i = 0; i < locks.size(); ++i) locks[i] &= ~((1 << 0) | (1 << 7));

    for (size_t i = 0; i < groups.size(); ++i)
    {
        // mark all remapped vertices as locked if seen by a prior group
        for (size_t j = 0; j < groups[i].size(); ++j)
        {
            const auto& cluster = clusters[groups[i][j]];

            for (size_t k = 0; k < cluster.indices.size(); ++k)
            {
                unsigned int v = cluster.indices[k];
                unsigned int r = remap[v];

                locks[r] |= locks[r] >> 7;
            }
        }

        // mark all remapped vertices as seen
        for (size_t j = 0; j < groups[i].size(); ++j)
        {
            const auto& cluster = clusters[groups[i][j]];

            for (size_t k = 0; k < cluster.indices.size(); ++k)
            {
                unsigned int v = cluster.indices[k];
                unsigned int r = remap[v];

                locks[r] |= 1 << 7;
            }
        }
    }

    for (size_t i = 0; i < locks.size(); ++i)
    {
        unsigned int r = remap[i];

        // consistently lock all vertices with the same position; keep protect bit if set
        locks[i] = (locks[r] & 1) | (locks[i] & meshopt_SimplifyVertex_Protect);
    }
}

void MClusterBuilder::ExtractClusterData(
        const InputData&                 input,
        const std::vector<MClusterData>& clusters,
        std::vector<MByte>&              outVertexData,
        std::vector<uint32_t>&           outIndexData,
        std::vector<uint32_t>&           outClusterIndices
)
{
    // Extract unique vertices used by this cluster and remap indices
    std::vector<uint32_t>                  uniqueVertices;
    std::unordered_map<uint32_t, uint32_t> vertexRemap;
    size_t                                 indicesNum = 0;
    for (const auto& cluster: clusters)
    {
        for (uint32_t idx: cluster.indices)
        {
            if (vertexRemap.find(idx) == vertexRemap.end())
            {
                uint32_t newIdx  = uniqueVertices.size();
                vertexRemap[idx] = newIdx;
                uniqueVertices.push_back(idx);
            }
        }

        indicesNum += cluster.indices.size();
    }

    // Copy vertex data
    const uint32_t vertexSize = input.vertexStride;
    outVertexData.resize(uniqueVertices.size() * vertexSize);
    for (size_t v = 0; v < uniqueVertices.size(); ++v)
    {
        const MByte* srcVertex = reinterpret_cast<const MByte*>(input.vertexData) + uniqueVertices[v] * vertexSize;
        MByte*       dstVertex = outVertexData.data() + v * vertexSize;
        memcpy(dstVertex, srcVertex, vertexSize);
    }

    // Remap and copy index data
    outIndexData.clear();
    outIndexData.reserve(indicesNum);
    outClusterIndices.resize(clusters.size());
    uint32_t indexOffset = 0;
    for (size_t idx = 0; idx < clusters.size(); ++idx)
    {
        const auto& cluster    = clusters[idx];
        outClusterIndices[idx] = indexOffset;

        for (uint32_t index: cluster.indices)
        {
            MORTY_ASSERT(vertexRemap.find(index) != vertexRemap.end());
            outIndexData.push_back(vertexRemap[index]);
        }

        indexOffset += static_cast<uint32_t>(cluster.indices.size());
    }
}

int32_t MClusterBuilder::OutputGroup(
        const InputData&                 input,
        const std::vector<MClusterData>& clusters,
        const std::vector<int>&          clusterInGroup,
        const MClusterBounds&            simplified
)
{
    int32_t groupId = m_allGroups.size();
    m_allGroups.push_back({});
    m_clusterPages.push_back({});

    MClusterGroup& group = m_allGroups[groupId];
    group.clusterOffset  = m_allClusters.size();
    group.clusterNum     = clusterInGroup.size();
    group.bounds         = simplified;

    // Initialize hierarchy fields
    group.parentGroupId     = MGlobal::M_INVALID_INT;
    group.firstChildGroupId = MGlobal::M_INVALID_INT;
    group.childGroupCount   = 0;

    std::vector<uint32_t> clusterIndices(clusterInGroup.size());
    // Extract cluster's independent vertex and index data
    ExtractClusterData(
            input,
            clusters,
            m_clusterPages[groupId].vertexData,
            m_clusterPages[groupId].indexData,
            clusterIndices
    );

    auto outputClusters = std::vector<MCluster>(clusterInGroup.size());
    for (size_t i = 0; i < clusterInGroup.size(); ++i)
    {
        const auto& cluster = clusters[clusterInGroup[i]];
        auto&       output  = outputClusters[i];

        output.refined = cluster.refined;
        output.group   = groupId;
        output.bounds =
                (OptimizeBounds && cluster.refined != -1)
                        ? BoundsCompute(input, cluster.indices.data(), cluster.indices.size(), cluster.bounds.error)
                        : cluster.bounds;

        output.indicesOffset = clusterIndices[i];
        output.indicesNum    = static_cast<uint32_t>(cluster.indices.size());
    }

    m_allClusters.insert(m_allClusters.end(), outputClusters.begin(), outputClusters.end());

    return groupId;
}

std::vector<MClusterBuilder::MClusterData> MClusterBuilder::Clusterize(const InputData& input)
{
    size_t                       max_meshlets = meshopt_buildMeshletsBound(input.indexNum, MaxVertices, MaxTriangles);
    std::vector<meshopt_Meshlet> meshlets(max_meshlets);
    std::vector<unsigned int>    meshlet_vertices(input.indexNum);
    // note: in v0.25 or prior, use indices.size() + max_meshlets * 3
    std::vector<unsigned char>   meshlet_triangles(input.indexNum + max_meshlets * 3);

    size_t                       meshlet_count = meshopt_buildMeshlets(
            meshlets.data(),
            meshlet_vertices.data(),
            meshlet_triangles.data(),
            input.indexData,
            input.indexNum,
            input.vertexData,
            input.vertexNum,
            input.vertexStride,
            MaxVertices,
            MaxTriangles,
            ConeWeight
    );

    if (meshlet_count > 0)
    {
        const meshopt_Meshlet& last = meshlets[meshlet_count - 1];

        meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
        meshlet_triangles.resize(last.triangle_offset + last.triangle_count * 3);
    }
    meshlets.resize(meshlet_count);

    auto clusters = std::vector<MClusterData>(meshlets.size());

    for (size_t i = 0; i < meshlets.size(); ++i)
    {
        const meshopt_Meshlet& meshlet = meshlets[i];

        clusters[i].vertexCount = meshlet.vertex_count;

        // note: we discard meshlet-local indices; they can be recovered by the caller using clodLocalIndices
        clusters[i].indices.resize(meshlet.triangle_count * 3);
        for (size_t j = 0; j < meshlet.triangle_count * 3; ++j)
            clusters[i].indices[j] =
                    meshlet_vertices[meshlet.vertex_offset + meshlet_triangles[meshlet.triangle_offset + j]];

        clusters[i].group   = -1;
        clusters[i].refined = -1;
    }

    return clusters;
}

MClusterBounds
MClusterBuilder::BoundsCompute(const InputData& input, const uint32_t* indices, uint32_t indicesNum, float error)
{
    meshopt_Bounds bounds =
            meshopt_computeClusterBounds(indices, indicesNum, input.vertexData, input.vertexNum, input.vertexStride);

    return MClusterBounds(Vector3(bounds.center[0], bounds.center[1], bounds.center[2]), bounds.radius, error);
}


MClusterBounds MClusterBuilder::BoundsMerge(const std::vector<MClusterData>& clusters, const std::vector<int>& group)
{
    std::vector<MClusterBounds> bounds(group.size());
    for (size_t j = 0; j < group.size(); ++j) bounds[j] = clusters[group[j]].bounds;

    meshopt_Bounds merged = meshopt_computeSphereBounds(
            bounds[0].position.m,
            bounds.size(),
            sizeof(MClusterBounds),
            &bounds[0].radius,
            sizeof(MClusterBounds)
    );

    MClusterBounds result = {};
    result.position.x     = merged.center[0];
    result.position.y     = merged.center[1];
    result.position.z     = merged.center[2];
    result.radius         = merged.radius;

    // merged bounds error must be conservative wrt cluster errors
    result.error = 0.f;
    for (size_t j = 0; j < group.size(); ++j) result.error = std::max(result.error, clusters[group[j]].bounds.error);


    return result;
}

void MClusterBuilder::SimplifyFallback(
        const InputData&                  input,
        std::vector<unsigned int>&        lod,
        const std::vector<unsigned int>&  indices,
        const std::vector<unsigned char>& locks,
        size_t                            target_count,
        float*                            error
)
{
    struct SloppyVertex {
        float        x, y, z;
        unsigned int id;
    };

    std::vector<SloppyVertex>  subset(indices.size());
    std::vector<unsigned char> subset_locks(indices.size());

    lod.resize(indices.size());

    size_t positions_stride = input.vertexStride / sizeof(float);

    // deindex the mesh subset to avoid calling simplifySloppy on the entire vertex buffer (which is prohibitively expensive without sparsity)
    for (size_t i = 0; i < indices.size(); ++i)
    {
        unsigned int v = indices[i];
        MORTY_ASSERT(v < input.vertexNum);

        subset[i].x  = input.vertexData[v * positions_stride + 0];
        subset[i].y  = input.vertexData[v * positions_stride + 1];
        subset[i].z  = input.vertexData[v * positions_stride + 2];
        subset[i].id = v;

        subset_locks[i] = locks[v];
        lod[i]          = unsigned(i);
    }

    lod.resize(meshopt_simplifySloppy(
            &lod[0],
            &lod[0],
            lod.size(),
            &subset[0].x,
            subset.size(),
            sizeof(SloppyVertex),
            subset_locks.data(),
            target_count,
            FLT_MAX,
            error
    ));

    // convert error to absolute
    *error *= meshopt_simplifyScale(&subset[0].x, subset.size(), sizeof(SloppyVertex));

    // restore original vertex indices
    for (size_t i = 0; i < lod.size(); ++i) lod[i] = subset[lod[i]].id;
}

std::vector<unsigned int> MClusterBuilder::Simplify(
        const InputData&                  input,
        const std::vector<unsigned int>&  indices,
        const std::vector<unsigned char>& locks,
        size_t                            target_count,
        float*                            error
)
{
    if (target_count > indices.size()) return indices;

    std::vector<unsigned int> lod(indices.size());

    unsigned int              options = meshopt_SimplifySparse | meshopt_SimplifyErrorAbsolute |
                           (SimplifyPermissive ? meshopt_SimplifyPermissive : 0);

    lod.resize(meshopt_simplifyWithAttributes(
            &lod[0],
            &indices[0],
            indices.size(),
            input.vertexData,
            input.vertexNum,
            input.vertexStride,
            input.vertexData,
            input.vertexStride,
            input.simplifyWeight.data(),
            input.simplifyWeight.size(),
            &locks[0],
            target_count,
            FLT_MAX,
            options,
            error
    ));

    // while it's possible to call simplifySloppy directly, it doesn't support sparsity or absolute error, so we need to do some extra work
    if (lod.size() > target_count && SimplifyFallbackSloppy)
    {
        SimplifyFallback(input, lod, indices, locks, target_count, error);
        *error *= SimplifyErrorFactorSloppy;// scale error up to account for appearance degradation
    }

    return lod;
}

void MClusterBuilder::BuildCluster(const InputData& input)
{
    if (input.vertexData == nullptr || input.vertexNum == 0 || input.indexData == nullptr || input.indexNum == 0)
    {
        return;
    }

    std::vector<unsigned char> locks(input.vertexNum);
    std::vector<unsigned int>  remap(input.vertexNum);

    meshopt_generatePositionRemap(&remap[0], input.vertexData, input.vertexNum, input.vertexStride);

    // set up protect bits on UV seams for permissive mode
    if (AttributeProtectMask)
    {
        size_t max_attributes = input.vertexStride / sizeof(float);

        for (size_t i = 0; i < input.vertexNum; ++i)
        {
            unsigned int r = remap[i];// canonical vertex with the same position

            for (size_t attributeIdx = 0; attributeIdx < max_attributes; ++attributeIdx)
                if (r != i && (input.attributeProtectMask & (1u << attributeIdx)) &&
                    input.vertexData[i * max_attributes + attributeIdx] !=
                            input.vertexData[r * max_attributes + attributeIdx])
                    locks[i] |= meshopt_SimplifyVertex_Protect;
        }
    }

    auto clusters = Clusterize(input);

    for (auto& cluster: clusters)
    {
        cluster.bounds = BoundsCompute(input, cluster.indices.data(), cluster.indices.size(), 0.0f);
    }


    std::vector<int> pending(clusters.size());
    for (size_t i = 0; i < clusters.size(); ++i) pending[i] = int(i);

    // merge and simplify clusters until we can't merge anymore
    while (pending.size() > 1)
    {
        std::vector<std::vector<int>> groups = PartitionCluster(input, clusters, pending, remap);
        pending.clear();

        m_lods.push_back(
                {.groupOffset = static_cast<uint32_t>(m_allGroups.size()),
                 .groupNum    = static_cast<uint32_t>(groups.size())}
        );

        // mark boundaries between groups with a lock bit to avoid gaps in simplified result
        LockBoundary(locks, groups, clusters, remap);

        // every group needs to be simplified now
        for (size_t i = 0; i < groups.size(); ++i)
        {
            std::vector<unsigned int> merged;
            for (size_t j = 0; j < groups[i].size(); ++j)
                merged.insert(
                        merged.end(),
                        clusters[groups[i][j]].indices.begin(),
                        clusters[groups[i][j]].indices.end()
                );

            size_t                    target_size = size_t((merged.size() / 3) * SimplifyRatio) * 3;

            // enforce bounds and error monotonicity
            // note: it is incorrect to use the precise bounds of the merged or simplified mesh, because this may violate monotonicity
            auto                      groupBounds = BoundsMerge(clusters, groups[i]);

            float                     error      = 0.f;
            std::vector<unsigned int> simplified = Simplify(input, merged, locks, target_size, &error);
            if (simplified.size() > size_t(merged.size() * SimplifyThreshold))
            {
                groupBounds.error = FLT_MAX;// terminal group, won't simplify further
                OutputGroup(input, clusters, groups[i], groupBounds);
                continue;// simplification is stuck; abandon the merge
            }

            // enforce error monotonicity (with an optional hierarchical factor to separate transitions more)
            groupBounds.error = std::max(groupBounds.error * SimplifyErrorMergePrevious, error) +
                                error * SimplifyErrorMergeAdditive;

            // output the new group with all clusters; the resulting id will be recorded in new clusters as clodCluster::refined
            auto    refined = OutputGroup(input, clusters, groups[i], groupBounds);

            // Get the parent group ID from the first cluster in the group
            int32_t parentGroupId = MGlobal::M_INVALID_INT;
            if (!groups[i].empty())
            {
                const auto& firstCluster = clusters[groups[i][0]];
                if (firstCluster.group != -1) { parentGroupId = firstCluster.group; }
            }

            // Update parent-child relationships
            if (parentGroupId != MGlobal::M_INVALID_INT)
            {
                MClusterGroup& parentGroup = m_allGroups[parentGroupId];

                // Set the first child if not already set
                if (parentGroup.firstChildGroupId == MGlobal::M_INVALID_INT)
                {
                    parentGroup.firstChildGroupId = refined;
                }

                // Increment child count
                parentGroup.childGroupCount++;

                // Set parent reference in the refined group
                m_allGroups[refined].parentGroupId = parentGroupId;
            }

            // discard clusters from the group - they won't be used anymore
            for (size_t j = 0; j < groups[i].size(); ++j) clusters[groups[i][j]].indices = std::vector<unsigned int>();

            InputData simplifiedInput       = input;
            simplifiedInput.indexData       = simplified.data();
            simplifiedInput.indexNum        = static_cast<uint32_t>(simplified.size());
            std::vector<MClusterData> split = Clusterize(simplifiedInput);

            for (auto& cluster: split)
            {
                cluster.refined = refined;
                cluster.group   = refined;// Set the group to the refined group ID

                // update cluster group bounds to the group-merged bounds; this ensures that we compute the group bounds for whatever group this cluster will be part of conservatively
                cluster.bounds = groupBounds;

                // enqueue new cluster for further processing
                clusters.push_back(std::move(cluster));
                pending.push_back(int(clusters.size()) - 1);
            }
        }
    }

    if (pending.size())
    {
        MORTY_ASSERT(pending.size() == 1);

        m_lods.push_back({.groupOffset = static_cast<uint32_t>(m_allGroups.size()), .groupNum = 1});

        const auto& cluster = clusters[pending[0]];

        auto        bounds = cluster.bounds;
        bounds.error       = FLT_MAX;// terminal group, won't simplify further

        auto    rootGroupId = OutputGroup(input, clusters, pending, bounds);

        // Update parent-child relationship for the root group
        int32_t parentGroupId = MGlobal::M_INVALID_INT;
        if (cluster.group != -1) { parentGroupId = cluster.group; }

        if (parentGroupId != MGlobal::M_INVALID_INT)
        {
            MClusterGroup& parentGroup = m_allGroups[parentGroupId];

            if (parentGroup.firstChildGroupId == MGlobal::M_INVALID_INT)
            {
                parentGroup.firstChildGroupId = rootGroupId;
            }

            parentGroup.childGroupCount++;
            m_allGroups[rootGroupId].parentGroupId = parentGroupId;
        }
    }
}
