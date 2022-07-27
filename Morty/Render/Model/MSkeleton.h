/**
 * @File         MSkeleton
 * 
 * @Created      2019-12-03 16:27:14
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSKELETON_H_
#define _M_MSKELETON_H_
#include "MGlobal.h"
#include "Matrix.h"
#include "MString.h"
#include "MVariant.h"
#include "MResource.h"
#include <vector>
#include <map>

class MBoundsOBB;
class MShaderParamSet;

class MORTY_API MBone
{
public:
	MBone();

	void WriteToStruct(MStruct& srt);
	void ReadFromStruct(const MStruct& srt);
public:
	MString strName;
	uint32_t unIndex;
	uint32_t unParentIndex;
	std::vector<uint32_t> vChildrenIndices;

	//Bones World
	Matrix4 m_matTransform;
	Matrix4 m_matOffsetMatrix;

	Matrix4 m_matWorldTransform;
};

class MORTY_API MSkeleton : public MResource
{
public:
	MORTY_CLASS(MSkeleton)
public:
	MSkeleton();
	virtual ~MSkeleton();
	
	const std::map<MString, uint32_t>& GetBonesMap() const { return m_tBonesMap; }

	void CopyAllBones(std::vector<MBone*>& allBones);

	MBone* FindBoneByName(const MString& strName);
	const MBone* FindBoneByName(const MString& strName) const;
	MBone* AppendBone(const MString& strName);
	void SortByDeep();

	void RebuildBonesMap();

	const std::vector<MBone>& GetAllBones() const { return m_vAllBones; }

public:

	void WriteToStruct(MStruct& srt);
	void ReadFromStruct(const MStruct& srt);

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;

	static MString GetResourceTypeName() { return "Skeleton"; };
	static std::vector<MString> GetSuffixList() { return {"ske"}; };

private:
	friend class MModelResource;
	friend class MSkeletonResource;

private:
	std::map<MString, uint32_t> m_tBonesMap;
	std::vector<MBone> m_vAllBones;
};

class MORTY_API MSkeletonInstance
{
public:
	MSkeletonInstance(std::shared_ptr<const MSkeleton> templateSke);
	MSkeletonInstance(const MSkeletonInstance& instance);
	~MSkeletonInstance();

	std::shared_ptr<const MSkeleton> GetSkeletonTemplate() const { return m_pSkeletonTemplate; }

	MBone* FindBoneByName(const MString& strName);

	const MBone* FindBoneTemplateByName(const MString& strName);
	const MBone* GetBoneTemplateByIndex(const uint32_t& unIndex);

	std::vector<MBone>& GetAllBones() { return m_vAllBones; }

	void ResetOriginPose();

	MShaderParamSet* GetShaderParamSet();

	void SetDirty();

private:
	MEngine* m_pEngine;
	std::shared_ptr<const MSkeleton> m_pSkeletonTemplate;
	std::vector<MBone> m_vAllBones;

	bool m_bShaderParamSetDirty;

	MShaderParamSet* m_pShaderParamSet;
	MVariantArray* m_pShaderBonesArray;
};

#endif
