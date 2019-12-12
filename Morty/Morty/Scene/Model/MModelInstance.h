/**
 * @File         MModelInstance
 * 
 * @Created      2019-08-29 20:54:27
 *
 * @Author       Morty
**/

#ifndef _M_MMODELINSTANCE_H_
#define _M_MMODELINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MResource;
class MModelResource;
class MSkeletonInstance;
class MIAnimController;
class MORTY_CLASS MModelInstance : public M3DNode
{
public:
	M_OBJECT(MModelInstance);
    MModelInstance();
    virtual ~MModelInstance();

public:

	bool Load(MResource* pResource);

	MModelResource* GetResource(){ return m_pResource; }
	MSkeletonInstance* GetSkeleton() { return m_pSkeleton; }
	
public:
	bool SetPlayAnimation(const MString& strAnimationName);
	MIAnimController* GetSkeletalAnimationController() { return m_pCurrentAnimationController; }

public:

	virtual void Tick(const float& fDelta) override;

private:

	MSkeletonInstance* m_pSkeleton;
	MModelResource* m_pResource;

	MIAnimController* m_pCurrentAnimationController;
};

#endif
