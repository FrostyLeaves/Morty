/**
 * @File         MSkeleton
 * 
 * @Created      2019-12-03 16:27:14
 *
 * @Author       Morty
**/

#ifndef _M_MSKELETON_H_
#define _M_MSKELETON_H_
#include "MGlobal.h"
#include "MString.h"
#include <vector>
#include <map>

class MORTY_CLASS MBone
{
public:
	static const unsigned int InvalidIndex;
public:
	MString strName;
	unsigned int unIndex;
	unsigned int unParentIndex;
	std::vector<unsigned int> vChildrenIndices;
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

private:
	std::map<MString, unsigned int> m_tBonesMap;
	std::vector<MBone*> m_vAllBones;
};

class MORTY_CLASS MSkeletonInstance
{
public:
	MSkeletonInstance(MSkeleton& templateSke);
	MSkeletonInstance(const MSkeletonInstance& instance);

	MBone* FindBoneByName(const MString& strName);

private:
	MSkeleton* m_pSkeletonTemplate;
	std::vector<MBone*> m_vAllBones;
};


#endif
