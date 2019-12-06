#include "MSkeleton.h"

const unsigned int MBone::InvalidIndex = -1;

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
	return pBone;
}

MSkeletonInstance::MSkeletonInstance(MSkeleton& templateSke)
	: m_pSkeletonTemplate(&templateSke)
{
	 m_pSkeletonTemplate->CopyAllBones(m_vAllBones);
}

MSkeletonInstance::MSkeletonInstance(const MSkeletonInstance& instance)
	: m_pSkeletonTemplate(instance.m_pSkeletonTemplate)
{
	m_pSkeletonTemplate->CopyAllBones(m_vAllBones);
}

MBone* MSkeletonInstance::FindBoneByName(const MString& strName)
{
	auto iter = m_pSkeletonTemplate->GetBonesMap().find(strName);
	if (iter != m_pSkeletonTemplate->GetBonesMap().cend())
	{
		return m_vAllBones[iter->second];
	}

	return nullptr;
}

