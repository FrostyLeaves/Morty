#include "Resource/MModelResource.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MModelResource_generated.h"
#include "Utility/MFileHelper.h"
#include "System/MResourceSystem.h"

MORTY_INTERFACE_IMPLEMENT(MModelResource, MResource)

MModelResource::MModelResource()
    : MResource()
    , m_skeleton(nullptr)
    , m_vMeshes()
{
}

MModelResource::~MModelResource()
{
}

void MModelResource::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeleton)
{
    m_skeleton.SetResource(pSkeleton);
}

void MModelResource::GetMeshResources(const std::vector<std::shared_ptr<MMeshResource>>& vMeshes)
{
    m_vMeshes = vMeshes;
}

void MModelResource::OnDelete()
{
    m_skeleton.SetResource(nullptr);

    m_vMeshes.clear();
}

flatbuffers::Offset<void> MModelResource::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto fbSkeletonResource = m_skeleton.Serialize(fbb);

    std::vector<flatbuffers::Offset<mfbs::MResourceRef>> vMeshResources;
    for (auto& mesh : m_vMeshes)
    {
        MResourceRef resource(mesh);
        vMeshResources.push_back(resource.Serialize(fbb).o);
    }

    const auto fbMeshResources = fbb.CreateVector(vMeshResources);

    mfbs::MModelResourceBuilder builder(fbb);

    builder.add_skeleton_resource(fbSkeletonResource.o);
    builder.add_mesh_resources(fbMeshResources.o);

    return builder.Finish().Union();
}

void MModelResource::Deserialize(const void* pBufferPointer)
{
    const mfbs::MModelResource* fbData = reinterpret_cast<const mfbs::MModelResource*>(pBufferPointer);
    m_skeleton.Deserialize(GetEngine(), fbData->skeleton_resource());

    m_vMeshes.clear();
    for (size_t idx = 0; idx < fbData->mesh_resources()->size(); ++idx)
    {
        MResourceRef resource;
        resource.Deserialize(GetEngine(), fbData->mesh_resources()->Get(idx));
        m_vMeshes.push_back(resource.GetResource<MMeshResource>());
    }
}

bool MModelResource::Load(const MString& strResourcePath)
{
    std::vector<MByte> data;
    MFileHelper::ReadData(strResourcePath, data);

    flatbuffers::FlatBufferBuilder fbb;
    fbb.PushBytes((const uint8_t*)data.data(), data.size());

    const mfbs::MModelResource* fbMeshResource = mfbs::GetMModelResource(fbb.GetCurrentBufferPointer());
    Deserialize(fbMeshResource);

    return true;
}

bool MModelResource::SaveTo(const MString& strResourcePath)
{
    flatbuffers::FlatBufferBuilder fbb;
    Serialize(fbb);

    std::vector<MByte> data(fbb.GetSize());
    memcpy(data.data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

    return MFileHelper::WriteData(strResourcePath, data);
}
