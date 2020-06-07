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
#include "MString.h"
#include "Matrix.h"
#include <vector>
#include <map>

class MBoundsOBB;

class MORTY_CLASS MBone
{
public:
	static const unsigned int InvalidIndex;

public:
	MBone();
public:
	MString strName;
	unsigned int unIndex;
	unsigned int unParentIndex;
	std::vector<unsigned int> vChildrenIndices;

	//Bones World
	Matrix4 m_matTransform;
	Matrix4 m_matOffsetMatrix;

	Matrix4 GetTransformInModelWorld() { return m_matTransform * m_matOffsetMatrix; }
};

class MORTY_CLASS MSkeleton
{
public:
public:
	MSkeleton();
	virtual ~MSkeleton();
	
	const std::map<MString, unsigned int>& GetBonesMap() const { return m_tBonesMap; }

	void CopyAllBones(std::vector<MBone*>& allBones);

	MBone* FindBoneByName(const MString& strName) const;
	MBone* AppendBone(const MString& strName);
	void SortByDeep();

	void RebuildBonesMap();

	const std::vector<MBone*>& GetAllBones() const { return m_vAllBones; }

private:
	friend class MModelResource;
	friend class MSkeletonResource;
	std::map<MString, unsigned int> m_tBonesMap;
	std::vector<MBone*> m_vAllBones;
};

class MORTY_CLASS MSkeletonInstance
{
public:
	MSkeletonInstance(const MSkeleton* templateSke);
	MSkeletonInstance(const MSkeletonInstance& instance);

	const MSkeleton* GetSkeletonTemplate() const { return m_pSkeletonTemplate; }

	MBone* FindBoneByName(const MString& strName);

	const MBone* FindBoneTemplateByName(const MString& strName);
	const MBone* GetBoneTemplateByIndex(const unsigned int& unIndex);

	const std::vector<MBone*>& GetAllBones() { return m_vAllBones; }

private:
	const MSkeleton* m_pSkeletonTemplate;
	std::vector<MBone*> m_vAllBones;
};

#endif
