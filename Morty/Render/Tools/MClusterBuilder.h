
#pragma once

#include "Utility/MGlobal.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertex.h"
#include "Resource/MMeshResource.h"

struct aiMesh;

namespace morty
{

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
            std::vector<MByte>&              outVertexData,
            std::vector<uint32_t>&           outIndexData,
            std::vector<uint32_t>&           outClusterIndices
    );

    void                         BuildCluster(const InputData& input);
    std::vector<MClusterData>    Clusterize(const InputData& input);

    const size_t                 MaxVertices  = 64;
    const size_t                 MaxTriangles = 128;// note: in v0.25 or prior, max_triangles needs to be divisible by 4
    const float                  ConeWeight   = 0.0f;
    const bool                   OptimizeBounds             = true;
    const bool                   AttributeProtectMask       = true;
    const bool                   PartitionSort              = true;
    const size_t                 PartitionSize              = 24;
    const bool                   SimplifyPermissive         = true;
    const bool                   SimplifyFallbackSloppy     = true;
    const float                  SimplifyRatio              = 0.5f;
    const float                  SimplifyThreshold          = 0.85f;
    const float                  SimplifyErrorFactorSloppy  = 2.0f;
    const float                  SimplifyErrorMergePrevious = 1.0f;
    const float                  SimplifyErrorMergeAdditive = 0.0f;


    void BuildChildRelationships();

    std::vector<MCluster>              m_allClusters;
    std::vector<MClusterGroup>         m_allGroups;
    std::vector<MClusterLodData>       m_lods;
    std::vector<MClusterPage>          m_clusterPages;
    std::vector<int32_t>               m_groupLinks;
    std::vector<std::vector<int32_t>>  m_groupParents;  // temporary storage for building child relationships
};

}// namespace morty