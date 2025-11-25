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
    int32_t                   group         = -1;
    int32_t                   refined       = -1;
    MClusterBounds            bounds;

    std::vector<MByte>      vertexData;
    std::vector<uint32_t>   indexData;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};

struct MClusterGroup {
    uint32_t                  clusterOffset;
    uint32_t                  clusterNum;
    MClusterBounds            bounds;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};

struct MClusterLodData {
    uint32_t                  groupOffset;
    uint32_t                  groupNum;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};

}// namespace morty
