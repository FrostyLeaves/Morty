/**
 * @File         MSkeletonInstance
 * 
 * @Created      2019-12-03 16:27:14
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSKELETON_INSTANCE_H_
#define _M_MSKELETON_INSTANCE_H_
#include "Utility/MGlobal.h"
#include "Math/Matrix.h"
#include "Object/MObject.h"
#include "Utility/MString.h"
#include "Variant/MVariant.h"
#include "Resource/MResource.h"
#include "MSkeleton.h"
#include <vector>
#include <map>

class MBoundsOBB;
class MSkeletonResource;
class MShaderPropertyBlock;

class MORTY_API MSkeletonInstance : public MObject
{
public:

	MORTY_CLASS(MSkeletonInstance);
	MSkeletonInstance() = default;
	~MSkeletonInstance() = default;

	void SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource);
	const MSkeleton* GetSkeletonTemplate() const { return m_pSkeletonTemplate; }

	MBone* FindBoneByName(const MString& strName);

	const MBone* FindBoneTemplateByName(const MString& strName);
	const MBone* GetBoneTemplateByIndex(const uint32_t& unIndex);

	std::vector<MBone>& GetAllBones() { return m_vAllBones; }

	void ResetOriginPose();

	std::shared_ptr<MShaderPropertyBlock> GetShaderParamSet();

	void SetDirty();

	void OnCreated() override;
	void OnDelete() override;

private:
	MResourceRef m_skeletonResource;
	const MSkeleton* m_pSkeletonTemplate = nullptr;
	std::vector<MBone> m_vAllBones;

	bool m_bShaderParamSetDirty = true;

	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	MVariantArray* m_pShaderBonesArray = nullptr;
};

#endif
