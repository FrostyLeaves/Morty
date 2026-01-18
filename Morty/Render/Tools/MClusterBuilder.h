
#pragma once

#include "Utility/MGlobal.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertex.h"
#include "Resource/MMeshResource.h"

struct aiMesh;

namespace morty
{

enum class MEClusterQuality
{
    Low,     // Aggressive simplification, fewer clusters
    Medium,  // Balanced quality and performance
    High,    // Conservative simplification, more clusters
    Ultra    // Highest quality, maximum clusters
};

class MClusterBuilder
{
public:
    struct InputData {
        float*             vertexData;
        uint32_t           vertexNum;
        uint32_t           vertexStride;

        uint32_t*          indexData;
        uint32_t           indexNum;

        size_t             attributeProtectMask;
        std::vector<float> simplifyWeight;
    };

    void SetQuality(MEClusterQuality quality);

    void Generate(MIMesh* mesh);

private:
    struct MClusterData {
        uint32_t              vertexCount;
        std::vector<uint32_t> indices;
        int32_t               group   = -1;
        int32_t               refined = -1;
        MClusterBounds        bounds;
    };

    std::vector<std::vector<int>> PartitionCluster(
            const InputData&                 input,
            const std::vector<MClusterData>& clusters,
            const std::vector<int>&          pending,
            const std::vector<unsigned int>& remap
    );

    void LockBoundary(
            std::vector<unsigned char>&          locks,
            const std::vector<std::vector<int>>& groups,
            const std::vector<MClusterData>&     clusters,
            const std::vector<unsigned int>&     remap
    );

    MClusterBounds BoundsCompute(const InputData& input, const uint32_t* indices, uint32_t indicesNum, float error);

    MClusterBounds BoundsMerge(const std::vector<MClusterData>& clusters, const std::vector<int>& group);

    void           SimplifyFallback(
                      const InputData&                  input,
                      std::vector<unsigned int>&        lod,
                      const std::vector<unsigned int>&  indices,
                      const std::vector<unsigned char>& locks,
                      size_t                            target_count,
                      float*                            error
              );

    std::vector<unsigned int> Simplify(
            const InputData&                  input,
            const std::vector<unsigned int>&  indices,
            const std::vector<unsigned char>& locks,
            size_t                            target_count,
            float*                            error
    );

    int32_t OutputGroup(
            const InputData&                 input,
            const std::vector<MClusterData>& clusters,
            const std::vector<int>&          group,
            const MClusterBounds&            simplified
    );

    void ExtractClusterData(
            const InputData&                 input,
            const std::vector<MClusterData>& clusters,
            const std::vector<int>&          clusterInGroup,
            std::vector<MByte>&              outVertexData,
            std::vector<uint32_t>&           outIndexData,
            std::vector<uint32_t>&           outClusterIndices
    );

    void                         BuildCluster(const InputData& input);
    std::vector<MClusterData>    Clusterize(const InputData& input);
    void                         CollectRootGroups();

    // Cluster building parameters (configurable via SetQuality)
    size_t m_maxVertices            = 64;
    size_t m_maxTriangles           = 128;  // note: in v0.25 or prior, max_triangles needs to be divisible by 4
    float  m_coneWeight             = 0.0f;
    bool   m_optimizeBounds         = true;
    bool   m_attributeProtectMask   = true;
    bool   m_partitionSort          = true;
    size_t m_partitionSize          = 24;
    bool   m_simplifyPermissive     = true;
    bool   m_simplifyFallbackSloppy = true;
    float  m_simplifyRatio          = 0.5f;
    float  m_simplifyThreshold      = 0.85f;
    float  m_simplifyErrorFactorSloppy  = 2.0f;
    float  m_simplifyErrorMergePrevious = 1.0f;
    float  m_simplifyErrorMergeAdditive = 0.0f;

    // Output data
    std::vector<MCluster>      m_allClusters;
    std::vector<MClusterGroup> m_allGroups;
    std::vector<MClusterPage>  m_clusterPages;
    std::vector<int32_t>       m_groupLinks;
    std::vector<uint32_t>      m_rootGroups;
};

}// namespace morty