/**
 * @File         MModelInstance
 * 
 * @Created      2019-08-29 20:54:27
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMODELINSTANCE_H_
#define _M_MMODELINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"
#include "MResource.h"

class MResource;
class MBoundsOBB;
class MBoundsAABB;
class MModelResource;
class MSkeletonInstance;
class MSkeletalAnimController;
class MORTY_CLASS MModelInstance : public M3DNode
{
public:
	M_OBJECT(MModelInstance);
    MModelInstance();
    virtual ~MModelInstance();

public:

	bool Load(MResource* pResource);

	MModelResource* GetResource();
	MSkeletonInstance* GetSkeleton() { return m_pSkeleton; }

	MBoundsAABB* GetBoundsAABB();

	virtual void SetVisible(const bool& bVisible) override;

	void SetDrawBoundingBox(const bool& bDrawable) { m_bDrawBoundingBox = bDrawable; }
	bool GetDrawBoundingBox() { return m_bDrawBoundingBox; }

public:
	bool SetPlayAnimation(const MString& strAnimationName);
	void SetRemoveAnimation();
	MSkeletalAnimController* GetSkeletalAnimationController() { return m_pCurrentAnimationController; }

public:

	virtual void Tick(const float& fDelta) override;



private:
	MBoundsAABB* m_pBoundsAABB;
	MSkeletonInstance* m_pSkeleton;
	MResourceHolder* m_pModelResource;

	MSkeletalAnimController* m_pCurrentAnimationController;

	bool m_bDrawBoundingBox;
};

#endif
