/**
 * @File         MModelInstance
 * 
 * @Created      2019-08-29 20:54:27
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMODELINSTANCE_H_
#define _M_MMODELINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"
#include "MResource.h"

class MSkeleton;
class MBoundsOBB;
class MBoundsAABB;
class MModelResource;
class MSkeletonInstance;
class MSkeletalAnimController;
class MORTY_API MModelInstance : public M3DNode
{
public:

public:
	M_OBJECT(MModelInstance);
    MModelInstance();
    virtual ~MModelInstance();

public:

	bool Load(MResource* pResource);
	void ClearSkeletonAndMesh();
	void Unload();

	MModelResource* GetResource();
	MString GetResourcePath() { return m_ModelResource.GetResourcePath(); }

	void SetSkeletonTemplate(MSkeleton* pSkeleton);
	void SetSkeletonTemplatePath(const MString& strSkeletonPath);
	MString GetSkeletonTemplatePath() const;
	
	MSkeletonInstance* GetSkeleton() { return m_pSkeleton; }


	MBoundsAABB* GetBoundsAABB();

	virtual void SetVisible(const bool& bVisible) override;

	void SetDrawBoundingBox(const bool& bDrawable) { m_bDrawBoundingBox = bDrawable; }
	bool GetDrawBoundingBox() { return m_bDrawBoundingBox; }

	void SetGenerateDirLightShadow(const bool& bGenerate) { m_bGenerateDirLightShadow = bGenerate; }
	bool GetGenerateDirLightShadow() const { return m_bGenerateDirLightShadow; }

public:
	bool SetPlayAnimation(MResource* pAnimation);
	bool SetPlayAnimation(const MString& strAnimationName);
	void SetRemoveAnimation();
	MSkeletalAnimController* GetSkeletalAnimationController() { return m_pCurrentAnimationController; }


public:

	virtual void Tick(const float& fDelta) override;
	virtual void OnDelete() override;

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;

protected:
	bool SetResourcePath(const MString& strResourcePath, const bool& bLoad = false);
	bool SetResource(MResource* pResource, const bool& bLoad = false);


private:
	MBoundsAABB* m_pBoundsAABB;
	MSkeletonInstance* m_pSkeleton;
	MResourceKeeper m_SkeletonResource;
	MResourceKeeper m_ModelResource;

	MSkeletalAnimController* m_pCurrentAnimationController;

	bool m_bDrawBoundingBox;
	bool m_bGenerateDirLightShadow;
};

#endif
