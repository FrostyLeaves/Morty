#include "MSkeletonResource.h"

#include "Flatbuffer/MSkeleton_generated.h"
#include "Utility/MFileHelper.h"

MORTY_CLASS_IMPLEMENT(MSkeletonResource, MResource)

MSkeleton* MSkeletonResource::GetSkeleton() const
{
    if (auto ptr = static_cast<MSkeletonResourceData*>(m_pResourceData.get()))
    {
        return &ptr->skeleton;
    }

    return nullptr;
}

bool MSkeletonResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
    m_pResourceData = std::move(pResourceData);

    return true;
}

bool MSkeletonResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    pResourceData = std::make_unique<MSkeletonResourceData>(*static_cast<MSkeletonResourceData*>(m_pResourceData.get()));

    return true;
}

flatbuffers::Offset<void> MSkeletonResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    return skeleton.Serialize(fbb);
}

void MSkeletonResourceData::Deserialize(const void* pBufferPointer)
{
    const auto fbResourceData = mfbs::GetMSkeleton(pBufferPointer);
    skeleton.Deserialize(fbResourceData);
}
