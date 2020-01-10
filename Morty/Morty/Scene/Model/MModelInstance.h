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

	MBoundsOBB* GetBoundsOBB();
	
public:
	bool SetPlayAnimation(const MString& strAnimationName);
	void SetRemoveAnimation();
	MSkeletalAnimController* GetSkeletalAnimationController() { return m_pCurrentAnimationController; }

public:

	virtual void Tick(const float& fDelta) override;

	virtual void SetVisible(const bool& bVisible) override;

private:

	MSkeletonInstance* m_pSkeleton;
	MResourceHolder* m_pModelResource;

	MSkeletalAnimController* m_pCurrentAnimationController;
};

#endif
