#pragma once

#include "Utility/MGlobal.h"
#include "BatchGroup/MStorageBatchGroup.h"
#include "Object/MObject.h"

namespace morty
{

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MSceneComponent;
class MShaderPropertyBlock;
struct MShaderConstantParam;

class MRenderMeshComponent;
class MORTY_API MMaterialBatchGroup
{
public:
    void                            Initialize(MEngine* pEngine, std::shared_ptr<MMaterialTemplate> pMaterial);

    void                            Release(MEngine* pEngine);

    static MMeshInstanceRenderProxy CreateProxyFromComponent(MRenderMeshComponent* pComponent);

    void                            AddMeshInstance(const MMeshInstanceRenderProxy& proxy);

    void                            UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy);

    void                            UpdateOrCreateMeshInstance(const MMeshInstanceRenderProxy& proxy);

    void                            RemoveMeshInstance(MMeshInstanceKey nProxyId);

    [[nodiscard]] bool              IsEmpty() const;

    [[nodiscard]] std::shared_ptr<MMaterialTemplate> GetMaterial() const { return m_materialTemplate; }

    [[nodiscard]] const MStorageBatchGroup*          GetInstanceBatchGroup() const { return &m_batchGroup; }

public:
    std::shared_ptr<MMaterialTemplate> m_materialTemplate = nullptr;
    MStorageBatchGroup                 m_batchGroup;
    MEngine*                           m_engine = nullptr;
};

}// namespace morty