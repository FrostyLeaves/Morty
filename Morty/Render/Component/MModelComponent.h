/**
 * @File         MModelComponent
 * 
 * @Created      2021-04-27 11:53:18
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMODELCOMPONENT_H_
#define _M_MMODELCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "MBounds.h"
#include "MResource.h"

class MSkeleton;
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

	void SetSkeletonResource(MSkeleton* pSkeleton);
	void SetSkeletonResourcePath(const MString& strSkeletonPath);
	MString GetSkeletonResourcePath() const;

	MSkeletonInstance* GetSkeleton() { return m_pSkeleton; }

public:

	bool PlayAnimation(MResource* pAnimation);
	bool PlayAnimation(const MString& strAnimationName);
	void RemoveAnimation();
	MSkeletalAnimController* GetSkeletalAnimationController();

public:

	MBoundsAABB GetBoundsAABB();

	bool GetBoundingBoxVisiable() const { return m_bBoundingBoxVisiable; }
	void SetBoundingBoxVisiable(const bool& bVisiable) { m_bBoundingBoxVisiable = bVisiable; }


public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb);
	virtual void Deserialize(const void* pBufferPointer);

private:

	MResourceKeeper m_SkeletonResource;

	MSkeletonInstance* m_pSkeleton;
	MSkeletalAnimController* m_pCurrentAnimationController;

	bool m_bBoundingBoxVisiable;
};


#endif
