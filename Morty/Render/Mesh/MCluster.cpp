#include "MCluster.h"
#include "Flatbuffer/MCluster_generated.h"

using namespace morty;

flatbuffers::Offset<void> MClusterBounds::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    fbs::MClusterBoundsBuilder builder(fbb);

    builder.add_position(position.Serialize(fbb));
    builder.add_radius(radius);
    builder.add_error(error);

    return builder.Finish().Union();
}

void MClusterBounds::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MClusterBounds*>(pBufferPointer);

    position.Deserialize(fbData->position());
    radius = fbData->radius();
    error  = fbData->error();
}

flatbuffers::Offset<void> MCluster::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto                 fbBounds = bounds.Serialize(fbb);

    fbs::MClusterBuilder builder(fbb);
    builder.add_bounds(fbBounds.o);
    builder.add_group(group);
    builder.add_indices_offset(indicesOffset);
    builder.add_indices_num(indicesNum);
    builder.add_refined(refined);

    return builder.Finish().Union();
}

void MCluster::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MCluster*>(pBufferPointer);
    bounds.Deserialize(fbData->bounds());
    group         = fbData->group();
    indicesOffset = fbData->indices_offset();
    indicesNum    = fbData->indices_num();
    refined       = fbData->refined();
}

flatbuffers::Offset<void> MClusterGroup::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto                      fbBounds = bounds.Serialize(fbb);

    fbs::MClusterGroupBuilder builder(fbb);
    builder.add_cluster_offset(clusterOffset);
    builder.add_cluster_num(clusterNum);
    builder.add_bounds(fbBounds.o);

    return builder.Finish().Union();
}

void MClusterGroup::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MClusterGroup*>(pBufferPointer);
    clusterOffset      = fbData->cluster_offset();
    clusterNum         = fbData->cluster_num();
    bounds.Deserialize(fbData->bounds());
}


flatbuffers::Offset<void> MClusterLodData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    fbs::MSliceBuilder builder(fbb);

    builder.add_offset(groupOffset);
    builder.add_num(groupNum);

    return builder.Finish().Union();
}

void MClusterLodData::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MSlice*>(pBufferPointer);
    groupOffset        = fbData->offset();
    groupNum           = fbData->num();
}