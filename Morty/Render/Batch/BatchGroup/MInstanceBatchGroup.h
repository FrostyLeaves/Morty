#pragma once

#include "Utility/MRenderGlobal.h"
#include "Component/MRenderMeshComponent.h"
#include "Object/MObject.h"
#include "Utility/MBounds.h"

namespace morty
{

class MShaderProgram;
struct MShaderConstantParam;

class MShaderPropertyBlock;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MRenderMeshComponent;
struct MORTY_API MMeshInstanceRenderProxy {
    bool                 bVisible       = true;
    bool                 bCullEnable    = true;
    MMeshInstanceKey     nProxyId       = MGlobal::M_INVALID_INDEX;
    MSkeletonInstanceKey nSkeletonId    = MGlobal::M_INVALID_INDEX;
    MIMesh*              pMesh          = nullptr;
    Matrix4              worldTransform = Matrix4::IdentityMatrix;
    MBoundsOBB           bounds;
    MBoundsAABB          boundsWithTransform;
};

class MORTY_API MInstanceBatchGroup
{
public:
    virtual ~MInstanceBatchGroup() = default;

    virtual void Initialize(MEngine* pEngine, std::shared_ptr<MShaderProgram> pShaderProgram) = 0;

    virtual void Release(MEngine* pEngine) = 0;

    virtual bool CanAddMeshInstance() const = 0;

    virtual void AddMeshInstance(const MMeshInstanceRenderProxy& proxy) = 0;

    virtual void RemoveMeshInstance(MMeshInstanceKey key) = 0;

    virtual void UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy) = 0;

    virtual std::shared_ptr<MShaderPropertyBlock> GetMeshProperty() const = 0;

    virtual MMeshInstanceRenderProxy*             FindMeshInstance(MMeshInstanceKey key) = 0;

    virtual void InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func) = 0;

    void         UpdateVisible(MMeshInstanceKey key, bool bVisible);
};

}// namespace morty