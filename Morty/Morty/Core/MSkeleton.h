/**
 * @File         MSkeleton
 * 
 * @Created      2019-12-03 16:27:14
 *
 * @Author       Pobrecito
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

class MORTY_CLASS MBone
{
public:
	MBone();

	void WriteToStruct(MStruct& srt);
	void ReadFromStruct(MStruct& srt);
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

class MORTY_CLASS MSkeleton : public MResource
{
public:
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
	void ReadFromStruct(MStruct& srt);

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;

private:
	friend class MModelResource;
	friend class MSkeletonResource;

private:
	std::map<MString, uint32_t> m_tBonesMap;
	std::vector<MBone> m_vAllBones;
};

class MORTY_CLASS MSkeletonInstance
{
public:
	MSkeletonInstance(const MSkeleton* templateSke);
	MSkeletonInstance(const MSkeletonInstance& instance);

	const MSkeleton* GetSkeletonTemplate() const { return m_pSkeletonTemplate; }

	MBone* FindBoneByName(const MString& strName);

	const MBone* FindBoneTemplateByName(const MString& strName);
	const MBone* GetBoneTemplateByIndex(const uint32_t& unIndex);

	std::vector<MBone>& GetAllBones() { return m_vAllBones; }

	void ResetOriginPose();

private:
	const MSkeleton* m_pSkeletonTemplate;
	std::vector<MBone> m_vAllBones;

	MShaderParamSet* m_pShaderParamSet;
};

#endif
