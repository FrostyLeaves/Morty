#include "MSkeleton.h"
#include <algorithm>

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
		(*allBones[i]) = (m_vAllBones[i]);
	}
}

MBone* MSkeleton::FindBoneByName(const MString& strName)
{
	auto iter = m_tBonesMap.find(strName);
	if (iter != m_tBonesMap.cend())
	{
		return &m_vAllBones[iter->second];
	}

	return nullptr;
}

const MBone* MSkeleton::FindBoneByName(const MString& strName) const
{
	auto iter = m_tBonesMap.find(strName);
	if (iter != m_tBonesMap.cend())
	{
		return &m_vAllBones[iter->second];
	}

	return nullptr;
}

MBone* MSkeleton::AppendBone(const MString& strName)
{
	m_vAllBones.push_back(MBone());
	MBone& bone = m_vAllBones.back();
	bone.strName = strName;
	bone.unIndex = m_vAllBones.size() - 1;
	bone.unParentIndex = M_INVALID_INDEX;
	m_tBonesMap[strName] = bone.unIndex;
	return &m_vAllBones.back();
}

void MSkeleton::SortByDeep()
{
	std::vector<int> map(m_vAllBones.size());
	std::map<unsigned int, int> tDeep;

	for (MBone& bone : m_vAllBones)
	{
		int deep = 0;
		unsigned int unParentIdx = bone.unIndex;
		while (unParentIdx != M_INVALID_INDEX)
		{
			unParentIdx = m_vAllBones[unParentIdx].unParentIndex;
			++deep;
		}

		tDeep[bone.unIndex] = deep;
	}

	std::vector<MBone>& vBones = m_vAllBones;

	std::sort(vBones.begin(), vBones.end(), [&tDeep](MBone& a, MBone& b) { return tDeep[a.unIndex] < tDeep[b.unIndex]; });

	for (unsigned int i = 0; i < vBones.size(); ++i)
		map[vBones[i].unIndex] = i;

	for (unsigned int i = 0; i < vBones.size(); ++i)
	{
		MBone& bone = vBones[i];
		bone.unIndex = map[bone.unIndex];
		if (M_INVALID_INDEX != bone.unParentIndex)
			bone.unParentIndex = map[bone.unParentIndex];
		for (unsigned int& index : bone.vChildrenIndices)
			index = map[index];
	}

	for (auto& iter : m_tBonesMap)
		iter.second = map[iter.second];

}

void MSkeleton::RebuildBonesMap()
{
	m_tBonesMap.clear();
	for (MBone& bone : m_vAllBones)
	{
		m_tBonesMap[bone.strName] = bone.unIndex;
	}
}

MSkeletonInstance::MSkeletonInstance(const MSkeleton* templateSke)
	: m_pSkeletonTemplate(templateSke)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();

	ResetOriginPose();
}

MSkeletonInstance::MSkeletonInstance(const MSkeletonInstance& instance)
	: m_pSkeletonTemplate(instance.m_pSkeletonTemplate)
{
	m_vAllBones = m_pSkeletonTemplate->GetAllBones();
}

MBone* MSkeletonInstance::FindBoneByName(const MString& strName)
{
	const MBone* pBoneTemp = m_pSkeletonTemplate->FindBoneByName(strName);
	if (nullptr == pBoneTemp)
		return nullptr;

	return &m_vAllBones[pBoneTemp->unIndex];
}

const MBone* MSkeletonInstance::FindBoneTemplateByName(const MString& strName)
{
	return m_pSkeletonTemplate->FindBoneByName(strName);
}

const MBone* MSkeletonInstance::GetBoneTemplateByIndex(const unsigned int& unIndex)
{
	return &m_pSkeletonTemplate->GetAllBones()[unIndex];
}

void MSkeletonInstance::ResetOriginPose()
{
	for (MBone& bone : m_vAllBones)
	{
		if (bone.unParentIndex != M_INVALID_INDEX)
		{
			bone.m_matWorldTransform = m_vAllBones[bone.unParentIndex].m_matWorldTransform * bone.m_matTransform;
		}
		else
		{
			bone.m_matWorldTransform = bone.m_matTransform;
		}
	}

	for (MBone& bone : m_vAllBones)
	{
		bone.m_matWorldTransform = bone.m_matWorldTransform * bone.m_matOffsetMatrix;
	}
}

