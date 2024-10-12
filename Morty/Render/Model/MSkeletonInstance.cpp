#include "Model/MSkeletonInstance.h"

#include "Engine/MEngine.h"
#include "Resource/MSkeletonResource.h"
#include "Shader/MShaderPropertyBlock.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSkeletonInstance, MObject)

void MSkeletonInstance::SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource)
{
    m_skeletonResource = pSkeletonRsource;

    if (auto pResource = m_skeletonResource.GetResource<MSkeletonResource>())
    {
        m_skeletonTemplate = pResource->GetSkeleton();
        if (m_skeletonTemplate)
        {
            m_allBones = m_skeletonTemplate->GetAllBones();
            ResetPose();
        }
    }
}

MBone* MSkeletonInstance::FindBoneByName(const MString& strName)
{
    const MBone* pBoneTemp = m_skeletonTemplate->FindBoneByName(strName);
    if (nullptr == pBoneTemp) return nullptr;

    return &m_allBones[pBoneTemp->unIndex];
}

const MBone* MSkeletonInstance::FindBoneTemplateByName(const MString& strName) const
{
    return m_skeletonTemplate->FindBoneByName(strName);
}

const MBone* MSkeletonInstance::GetBoneTemplateByIndex(const uint32_t& unIndex) const
{
    return &m_skeletonTemplate->GetAllBones()[unIndex];
}

std::vector<MBone>& MSkeletonInstance::GetAllBones() { return m_allBones; }

void                MSkeletonInstance::ResetPose()
{
    size_t nBonesSize = m_allBones.size();
    m_currentPose.vBoneMatrix.resize(nBonesSize);

    for (size_t nIdx = 0; nIdx < nBonesSize; ++nIdx) { m_currentPose.vBoneMatrix[nIdx] = Matrix4::IdentityMatrix; }
}