#include "Model/MSkeletonInstance.h"

#include "Engine/MEngine.h"
#include "Shader/MShaderPropertyBlock.h"
#include "Resource/MSkeletonResource.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSkeletonInstance, MObject)

void MSkeletonInstance::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource)
{
	m_skeletonResource = pSkeletonRsource;

	if (auto pResource = m_skeletonResource.GetResource<MSkeletonResource>())
	{
		m_pSkeletonTemplate = pResource->GetSkeleton();
		if (m_pSkeletonTemplate)
		{
			m_vAllBones = m_pSkeletonTemplate->GetAllBones();
			ResetPose();
		}
	}
}

MBone* MSkeletonInstance::FindBoneByName(const MString& strName)
{
	const MBone* pBoneTemp = m_pSkeletonTemplate->FindBoneByName(strName);
	if (nullptr == pBoneTemp)
		return nullptr;

	return &m_vAllBones[pBoneTemp->unIndex];
}
 
const MBone* MSkeletonInstance::FindBoneTemplateByName(const MString& strName) const
{
	return m_pSkeletonTemplate->FindBoneByName(strName);
}

const MBone* MSkeletonInstance::GetBoneTemplateByIndex(const uint32_t& unIndex) const
{
	return &m_pSkeletonTemplate->GetAllBones()[unIndex];
}

std::vector<MBone>& MSkeletonInstance::GetAllBones()
{
	return m_vAllBones;
}

void MSkeletonInstance::ResetPose()
{
	size_t nBonesSize = m_vAllBones.size();
	m_currentPose.vBoneMatrix.resize(nBonesSize);

	for (size_t nIdx = 0; nIdx < nBonesSize; ++nIdx)
	{
		m_currentPose.vBoneMatrix[nIdx] = Matrix4::IdentityMatrix;
	}
}