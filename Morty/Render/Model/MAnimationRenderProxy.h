/**
 * @File         MAnimationRenderProxy
 * 
 * @Created      2019-12-09 22:37:59
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MRenderGlobal.h"
#include "Basic/MStorageVariant.h"
#include "Batch/BatchGroup/MRenderInstanceCache.h"
#include "Component/MComponent.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "Variant/MVariant.h"

namespace morty
{

class MIDevice;
class MSkeletonInstance;
class MEngine;
class MShaderPropertyBlock;
struct MORTY_API MPoseRenderInstance {
    size_t     nMatrixOffset = 0;
    MemoryInfo bonesMemoryInfo;
};

struct MORTY_API MAnimationBufferData {
    const MBuffer* pBonesBuffer  = nullptr;
    const MBuffer* pOffsetBuffer = nullptr;
};

struct MORTY_API MPoseRenderProxy {
    std::vector<Matrix4> vBoneMatrix;
};

class MORTY_API MAnimationRenderGroup : public IPropertyBlockAdapter
{
public:
    void                 Initialize(MEngine* pEngine);

    void                 Release(MEngine* pEngine);

    void                 AddSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy);

    void                 RemoveSkeletonRenderInstance(MSkeletonInstanceKey nProxyId);

    void                 UpdateSkeletonRenderInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy);

    void                 UpdateOrCreateMeshInstance(MSkeletonInstanceKey nProxyId, const MPoseRenderProxy& poseProxy);

    MAnimationBufferData GetAnimationBuffer() const;

    std::shared_ptr<MShaderPropertyBlock> GetAnimationProperty() const { return m_shaderPropertyBlock; }

    std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const override { return GetAnimationProperty(); }

    static MPoseRenderProxy               CreatePoseProxy(MSkeletonInstance* pSkeletonInstance);

    MEngine*                              GetEngine() const { return m_engine; }

private:
    MPoseRenderInstance                                             CreatePoseRenderInstance();

    MEngine*                                                        m_engine = nullptr;

    MRenderInstanceCache<MSkeletonInstanceKey, MPoseRenderInstance> m_poseRenderInstance;
    MStorageVariant                                                 m_bonesStorageBuffer;
    MStorageVariant                                                 m_bonesOffsetBuffer;
    MMemoryPool                                                     m_bonesStorageMemoryPool = MMemoryPool(0);

    std::shared_ptr<MShaderPropertyBlock>                           m_shaderPropertyBlock = nullptr;
};

}// namespace morty