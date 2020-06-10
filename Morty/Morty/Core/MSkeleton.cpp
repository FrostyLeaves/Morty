#include "MSkeleton.h"
#include <algorithm>

const unsigned int MBone::InvalidIndex = -1;

MBone::MBone()
	: m_matTransform(Matrix4::IdentityMatrix)
	, m_matOffsetMatrix(Matrix4::IdentityMatrix)
	, m_matWorldTransform(Matrix4::IdentityMatrix)
{

}

MSkeleton::MSkeleton()
	: m_tBonesMap()
	, m_vAllBones()
{

}

MSkeleton::~MSkeleton()
{

}

void MSkeleton::CopyAllBones(std::vector<MBone*>& allBones)
{
	for (MBone* pBone : allBones)
		delete pBone;

	allBones.resize(m_vAllBones.size());

	for (int i = 0; i < m_vAllBones.size(); ++i)
	{
		allBones[i] = new MBone();
		(*allBones[i]) = (*m_vAllBones[i]);
	}
}

MBone* MSkeleton::FindBoneByName(const MString& strName) const
{
	auto iter = m_tBonesMap.find(strName);
	if (iter != m_tBonesMap.cend())
	{
		return m_vAllBones[iter->second];
	}

	return nullptr;
}

MBone* MSkeleton::AppendBone(const MString& strName)
{
	MBone* pBone = new MBone();
	m_vAllBones.push_back(pBone);
	pBone->strName = strName;
	pBone->unIndex = m_vAllBones.size() - 1;
	pBone->unParentIndex = MBone::InvalidIndex;
	m_tBonesMap[strName] = pBone->unIndex;
	return pBone;
}

void MSkeleton::SortByDeep()
{
	std::vector<int> map(m_vAllBones.size());
	std::vector<MBone*> vBones = m_vAllBones;
	std::map<MBone*, int> tDeep;

	for (MBone* pBone : m_vAllBones)
	{
		int deep = 0;
		MBone* pParent = pBone;
		while (pParent->unParentIndex != MBone::InvalidIndex)
		{
			pParent = m_vAllBones[pParent->unParentIndex];
			++deep;
		}

		tDeep[pBone] = deep;
	}

	std::sort(vBones.begin(), vBones.end(), [&tDeep](MBone* a, MBone* b) { return tDeep[a] < tDeep[b]; });

	for (unsigned int i = 0; i < vBones.size(); ++i)
		map[vBones[i]->unIndex] = i;

	for (unsigned int i = 0; i < vBones.size(); ++i)
	{
		MBone* pBone = vBones[i];
		pBone->unIndex = map[pBone->unIndex];
		if (MBone::InvalidIndex != pBone->unParentIndex)
			pBone->unParentIndex = map[pBone->unParentIndex];
		for (unsigned int& index : pBone->vChildrenIndices)
			index = map[index];
	}

	for (auto& iter : m_tBonesMap)
		iter.second = map[iter.second];

	m_vAllBones = vBones;
}

void MSkeleton::RebuildBonesMap()
{
	m_tBonesMap.clear();
	for (MBone* pBone : m_vAllBones)
	{
		m_tBonesMap[pBone->strName] = pBone->unIndex;
	}
}

MSkeletonInstance::MSkeletonInstance(const MSkeleton* templateSke)
	: m_pSkeletonTemplate(templateSke)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();
}

MSkeletonInstance::MSkeletonInstance(const MSkeletonInstance& instance)
	: m_pSkeletonTemplate(instance.m_pSkeletonTemplate)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();
}

MBone* MSkeletonInstance::FindBoneByName(const MString& strName)
{
	MBone* pBoneTemp = m_pSkeletonTemplate->FindBoneByName(strName);
	if (nullptr == pBoneTemp)
		return nullptr;

	return m_vAllBones[pBoneTemp->unIndex];
}

const MBone* MSkeletonInstance::FindBoneTemplateByName(const MString& strName)
{
	return m_pSkeletonTemplate->FindBoneByName(strName);
}

const MBone* MSkeletonInstance::GetBoneTemplateByIndex(const unsigned int& unIndex)
{
	return m_pSkeletonTemplate->GetAllBones()[unIndex];
}

