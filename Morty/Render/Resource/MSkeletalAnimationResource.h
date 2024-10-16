/**
 * @File         MSkeletalAnimationResource
 * 
 * @Created      2020-06-15 21:09:00
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Model/MSkeletalAnimation.h"
#include "Resource/MResource.h"
#include "Resource/MResourceLoader.h"
#include "Resource/MSkeletonResource.h"

namespace morty
{

struct MORTY_API MSkeletalAnimationResourceData : public MFbResourceData {
public:
    MSkeletalAnimation        skeletonAnimation;
    MPath                     skeletonResource;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;

    void                      Deserialize(const void* pBufferPointer) override;
};

class MORTY_API MSkeletalAnimationResource : public MResource
{
public:
    MORTY_CLASS(MSkeletalAnimationResource);

    MSkeletalAnimationResource() = default;

    virtual ~MSkeletalAnimationResource() = default;

    MString                   GetAnimationName() const;

    MSkeleton*                GetSkeleton() const;

    const MSkeletalAnimation* GetAnimation() const;

    void                      SetSkeletonResource(std::shared_ptr<MSkeletonResource> pResource);

    bool                      Load(std::unique_ptr<MResourceData>&& pResourceData) override;

    bool                      SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

private:
    MSkeletalAnimation m_skeletonAnimation;
    MResourceRef       m_skeletonResource;
};

class MORTY_API MSkeletalAnimationLoader
    : public MResourceLoaderTemplate<MSkeletalAnimationResource, MSkeletalAnimationResourceData>
{
public:
    static MString              GetResourceTypeName() { return "Animation"; };

    static std::vector<MString> GetSuffixList() { return {"anim"}; };
};

}// namespace morty