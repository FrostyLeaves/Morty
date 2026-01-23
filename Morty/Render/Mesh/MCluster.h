/**
* @File         MCluster
 *
 * @Created      2025-11-18 13:34:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MBuffer.h"
#include "Math/Vector.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

namespace morty
{

struct MClusterBounds {
    Vector3                   position;
    float                     radius;
    float                     error;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};

struct MCluster {
    int32_t                   group         = MGlobal::M_INVALID_INT;
    int32_t                   refined       = MGlobal::M_INVALID_INT;
    uint32_t                  indicesOffset = 0;//offset in MClusterGroup index array
    uint32_t                  indicesNum    = 0;//number of indices in MClusterGroup index array
    MClusterBounds            bounds;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};

struct MClusterPage {
    std::vector<MByte>        vertexData;
    std::vector<uint32_t>     indexData;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};

struct MClusterGroup {
    uint32_t                  clusterOffset;
    uint32_t                  clusterNum;
    MClusterBounds            bounds;

    // DAG structure: offset and count into global group links array
    uint32_t                  groupLinkOffset = 0;
    uint32_t                  groupLinkCount  = 0;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};


SHADER_STRUCT MClusterRenderData
{
    Vector3  position;
    float    radius;
    float    error;
    uint32_t indexOffset = 0;
    uint32_t indexCount  = 0;
    uint32_t valid       = 0;
};

// Mesh resource level data, used for finding ClusterGroup root nodes during GPU culling
SHADER_STRUCT MMeshResourceRenderData
{
    int32_t clusterBeginIndex      = 0;
    int32_t clusterGroupBeginIndex = 0;
    int32_t rootClusterGroupIndex  = 0;
    int32_t rootClusterGroupCount  = 0;
};

SHADER_STRUCT MMeshRenderData
{
    uint32_t rootClusterOffset = MGlobal::M_INVALID_UINDEX;
    uint32_t rootClusterCount  = 0;
};

SHADER_STRUCT MClusterGroupRenderData
{
    Vector3  position;
    float    radius;

    float    error;
    uint32_t valid = false;
    uint32_t groupLinkOffset;
    uint32_t groupLinkCount;

    uint32_t clusterBeginIndex;
    uint32_t clusterCount;
    uint32_t vertexBeginIndex;
    uint32_t indexBeginIndex;
};

struct MClusterPageMemoryData {
    MemoryInfo vertexMemoryInfo;
    MemoryInfo indexMemoryInfo;
    bool       loaded = false;
};


}// namespace morty
