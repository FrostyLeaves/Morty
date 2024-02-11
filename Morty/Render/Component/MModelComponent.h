/**
 * @File         MModelComponent
 * 
 * @Created      2021-04-27 11:53:18
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

#include "Utility/MBounds.h"
#include "Resource/MResource.h"
#include "Resource/MSkeletonResource.h"

MORTY_SPACE_BEGIN

class MSkeletonInstance;
class MSkeletalAnimController;
class MORTY_API MModelComponent : public MComponent
{
public:
    MORTY_CLASS(MModelComponent)
public:
    MModelComponent();
    virtual ~MModelComponent();

public:

	void Release() override;

	void SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource);
	void SetSkeletonResourcePath(const MString& strSkeletonPath);
	MString GetSkeletonResourcePath() const;

	MSkeletonInstance* GetSkeleton() const { return m_pSkeleton; }

public:

	bool PlayAnimation(std::shared_ptr<MResource> pAnimation);
	bool PlayAnimation(const MString& strAnimationName);
	void RemoveAnimation();
	MSkeletalAnimController* GetSkeletalAnimationController();

public:

	bool GetBoundingBoxVisiable() const { return m_bBoundingBoxVisiable; }
	void SetBoundingBoxVisiable(const bool& bVisiable) { m_bBoundingBoxVisiable = bVisiable; }


public:


	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	void Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;
	void Deserialize(const void* pBufferPointer) override;

private:

	MResourceRef m_SkeletonResource;

	MSkeletonInstance* m_pSkeleton = nullptr;
	MSkeletalAnimController* m_pCurrentAnimationController;

	bool m_bBoundingBoxVisiable;
};

MORTY_SPACE_END