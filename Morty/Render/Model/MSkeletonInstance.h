/**
 * @File         MSkeletonInstance
 * 
 * @Created      2019-12-03 16:27:14
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MSkeleton.h"
#include "Math/Matrix.h"
#include "Object/MObject.h"
#include "Resource/MResource.h"
#include "Utility/MString.h"
#include "Variant/MVariant.h"

namespace morty
{

class MBoundsOBB;
class MSkeletonResource;
class MShaderPropertyBlock;
class MORTY_API MSkeletonInstance : public MObject
{
public:
    MORTY_CLASS(MSkeletonInstance);

    MSkeletonInstance() = default;

    ~MSkeletonInstance() = default;

    void                SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeletonRsource);

    const MSkeleton*    GetSkeletonTemplate() const { return m_skeletonTemplate; }

    MBone*              FindBoneByName(const MString& strName);

    const MBone*        FindBoneTemplateByName(const MString& strName) const;

    const MBone*        GetBoneTemplateByIndex(const uint32_t& unIndex) const;

    std::vector<MBone>& GetAllBones();

    void                ResetPose();

    MSkeletonPose&      GetCurrentPose() { return m_currentPose; }

private:
    MResourceRef       m_skeletonResource;
    const MSkeleton*   m_skeletonTemplate = nullptr;
    std::vector<MBone> m_allBones;

    MSkeletonPose      m_currentPose;
};

}// namespace morty