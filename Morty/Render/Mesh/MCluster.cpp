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
    builder.add_refined(refined);
    builder.add_indices_offset(indicesOffset);
    builder.add_indices_num(indicesNum);

    return builder.Finish().Union();
}

void MCluster::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MCluster*>(pBufferPointer);
    bounds.Deserialize(fbData->bounds());
    group         = fbData->group();
    refined       = fbData->refined();
    indicesOffset = fbData->indices_offset();
    indicesNum    = fbData->indices_num();
}

flatbuffers::Offset<void> MClusterPage::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto fbVertex = fbb.CreateVector(reinterpret_cast<const int8_t*>(vertexData.data()), vertexData.size());
    auto fbIndex  = fbb.CreateVector(indexData);

    fbs::MClusterPageBuilder builder(fbb);
    builder.add_vertex(fbVertex);
    builder.add_index(fbIndex);
    return builder.Finish().Union();
}

void MClusterPage::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MClusterPage*>(pBufferPointer);

    if (fbData->vertex())
    {
        const auto* data = reinterpret_cast<const MByte*>(fbData->vertex()->data());
        vertexData.assign(data, data + fbData->vertex()->size());
    }
    if (fbData->index()) { indexData.assign(fbData->index()->begin(), fbData->index()->end()); }
}

flatbuffers::Offset<void> MClusterGroup::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto                      fbBounds = bounds.Serialize(fbb);

    fbs::MClusterGroupBuilder builder(fbb);
    builder.add_cluster_offset(clusterOffset);
    builder.add_cluster_num(clusterNum);
    builder.add_bounds(fbBounds.o);
    builder.add_group_link_offset(groupLinkOffset);
    builder.add_group_link_count(groupLinkCount);

    return builder.Finish().Union();
}

void MClusterGroup::Deserialize(const void* pBufferPointer)
{
    const auto* fbData = reinterpret_cast<const fbs::MClusterGroup*>(pBufferPointer);
    clusterOffset      = fbData->cluster_offset();
    clusterNum         = fbData->cluster_num();
    bounds.Deserialize(fbData->bounds());
    groupLinkOffset = fbData->group_link_offset();
    groupLinkCount  = fbData->group_link_count();
}


