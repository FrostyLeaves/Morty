#include "MSkeletalAnimationResource.h"

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"

#include "Resource/MSkeletonResource.h"
#include "Flatbuffer/MSkeletalAnimationResource_generated.h"

MORTY_CLASS_IMPLEMENT(MSkeletalAnimationResource, MResource);

flatbuffers::Offset<void> MSkeletalAnimationResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto fbAnimation = skeletonAnimation.Serialize(fbb).o;
    auto fbSkeleton = fbb.CreateString(skeletonResource).o;

    mfbs::MSkeletalAnimationResourceBuilder builder(fbb);

    builder.add_animation(fbAnimation);
    builder.add_skeleton(fbSkeleton);

    return builder.Finish().Union();
}

void MSkeletalAnimationResourceData::Deserialize(const void* pBufferPointer)
{
    auto fbResourceData = mfbs::GetMSkeletalAnimationResource(pBufferPointer);
    skeletonAnimation.Deserialize(fbResourceData->animation());
    skeletonResource = fbResourceData->skeleton()->str();
}

MString MSkeletalAnimationResource::GetAnimationName() const
{
    return m_skeletonAnimation.GetName();
}

MSkeleton* MSkeletalAnimationResource::GetSkeleton() const
{
    if (const auto pResource = m_pSkeletonResource.GetResource<MSkeletonResource>())
    {
        return pResource->GetSkeleton();
    }

    return nullptr;
}

const MSkeletalAnimation* MSkeletalAnimationResource::GetAnimation() const
{
    return &m_skeletonAnimation;
}

void MSkeletalAnimationResource::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonResource)
{
    m_pSkeletonResource = pSkeletonResource;
    m_skeletonAnimation.SetSkeletonTemplate(pSkeletonResource->GetSkeleton());
}

bool MSkeletalAnimationResource::Load(std::unique_ptr<MResourceData>& pResourceData)
{
    auto pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    if (auto pAnimationData = static_cast<MSkeletalAnimationResourceData*>(pResourceData.get()))
    {
        m_skeletonAnimation = pAnimationData->skeletonAnimation;

        const auto pResource = pResourceSystem->LoadResource(pAnimationData->skeletonResource);
        if (const auto pSkeletonResource = MTypeClass::DynamicCast<MSkeletonResource>(pResource))
        {
            SetSkeletonResource(pSkeletonResource);
        }
    }

    return true;
}

bool MSkeletalAnimationResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    auto pAnimationData = std::make_unique<MSkeletalAnimationResourceData>();
    pAnimationData->skeletonAnimation = m_skeletonAnimation;
    if (auto pResource = m_pSkeletonResource.GetResource<MSkeletonResource>())
    {
        pAnimationData->skeletonResource = pResource->GetResourcePath();
    }

    pResourceData = std::move(pAnimationData);
    return true;
}
