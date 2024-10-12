#pragma once

#include "Utility/MGlobal.h"
#include "BatchGroup/MInstanceBatchGroup.h"
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
    void                                     Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial);

    void                                     Release(MEngine* pEngine);

    static MMeshInstanceRenderProxy          CreateProxyFromComponent(MRenderMeshComponent* pComponent);

    void                                     AddMeshInstance(const MMeshInstanceRenderProxy& proxy);

    void                                     UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy);

    void                                     UpdateOrCreateMeshInstance(const MMeshInstanceRenderProxy& proxy);

    void                                     RemoveMeshInstance(MMeshInstanceKey nProxyId);

    bool                                     IsEmpty() const;

    std::shared_ptr<MMaterial>               GetMaterial() const { return m_material; }

    const std::vector<MInstanceBatchGroup*>& GetInstanceBatchGroup() const { return m_batchGroup; }


public:
    std::shared_ptr<MMaterial>         m_material = nullptr;
    std::map<MMeshInstanceKey, size_t> m_meshInstanceTable;
    std::vector<MInstanceBatchGroup*>  m_batchGroup;

    MEngine*                           m_engine = nullptr;
};

}// namespace morty