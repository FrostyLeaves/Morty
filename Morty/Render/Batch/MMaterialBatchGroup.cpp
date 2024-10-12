#include "MMaterialBatchGroup.h"

#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Scene/MEntity.h"
#include "System/MRenderSystem.h"

#include "BatchGroup/MNoneBatchGroup.h"
#include "BatchGroup/MStorageBatchGroup.h"
#include "BatchGroup/MUniformBatchGroup.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Mesh/MVertex.h"

using namespace morty;

class MORTY_API MMaterialBatchUtil
{
public:
    enum MORTY_API TransformType
    {
        ENone          = 0,
        EUniformArray  = 1,
        EStorageBuffer = 2
    };


    static TransformType        GetMaterialBatchTransformType(MMaterial* pMaterial);

    static MInstanceBatchGroup* CreateBatchGroup(MMaterial* pMaterial);
};


void MMaterialBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
{
    m_engine   = pEngine;
    m_material = pMaterial;
}

void MMaterialBatchGroup::Release(MEngine* pEngine)
{
    for (auto pMeshGroup: m_batchGroup)
    {
        pMeshGroup->Release(pEngine);
        delete pMeshGroup;
    }
    m_batchGroup.clear();
}

MMeshInstanceRenderProxy MMaterialBatchGroup::CreateProxyFromComponent(MRenderMeshComponent* pComponent)
{
    MMeshInstanceRenderProxy proxy;
    proxy.bVisible    = true;
    proxy.bCullEnable = pComponent->GetSceneCullEnable();
    proxy.nProxyId    = static_cast<uint32_t>(pComponent->GetComponentID().nIdx);
    proxy.nSkeletonId = static_cast<uint32_t>(pComponent->GetAttachedModelComponentID().nIdx);
    if (MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>())
    {
        proxy.worldTransform = pSceneComponent->GetWorldTransform();
    }
    else { proxy.worldTransform = Matrix4::IdentityMatrix; }

    if (auto pMeshResource = pComponent->GetMeshResource().GetResource<MMeshResource>())
    {
        proxy.pMesh  = pMeshResource->GetMesh();
        proxy.bounds = *pMeshResource->GetMeshesDefaultOBB();
        proxy.boundsWithTransform
                .SetBoundsOBB(proxy.worldTransform.GetTranslation(), proxy.worldTransform, proxy.bounds);
    }

    return proxy;
}

void MMaterialBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    const auto findResult = m_meshInstanceTable.find(proxy.nProxyId);
    if (findResult != m_meshInstanceTable.end())
    {
        MORTY_ASSERT(false);
        return;
    }

    size_t               nMeshGroupIdx = 0;
    MInstanceBatchGroup* pMeshGroup    = nullptr;
    for (size_t nIdx = 0; nIdx < m_batchGroup.size(); ++nIdx)
    {
        MInstanceBatchGroup* pCurrentGroup = m_batchGroup[nIdx];
        if (pCurrentGroup->CanAddMeshInstance())
        {
            nMeshGroupIdx = nIdx;
            pMeshGroup    = pCurrentGroup;
            break;
        }
    }
    if (pMeshGroup == nullptr)
    {
        pMeshGroup = MMaterialBatchUtil::CreateBatchGroup(m_material.get());
        pMeshGroup->Initialize(m_engine, m_material->GetShaderProgram());
        m_batchGroup.push_back(pMeshGroup);
        nMeshGroupIdx = m_batchGroup.size() - 1;
    }

    if (!pMeshGroup->CanAddMeshInstance())
    {
        MORTY_ASSERT(false);
        return;
    }

    pMeshGroup->AddMeshInstance(proxy);
    m_meshInstanceTable[proxy.nProxyId] = nMeshGroupIdx;
}

void MMaterialBatchGroup::RemoveMeshInstance(MMeshInstanceKey nProxyId)
{
    const auto findResult = m_meshInstanceTable.find(nProxyId);
    if (findResult == m_meshInstanceTable.end())
    {
        MORTY_ASSERT(false);
        return;
    }

    size_t nIdx = findResult->second;
    m_meshInstanceTable.erase(findResult);

    if (nIdx >= m_batchGroup.size())
    {
        MORTY_ASSERT(nIdx < m_batchGroup.size());
        return;
    }

    m_batchGroup[nIdx]->RemoveMeshInstance(nProxyId);
}

void MMaterialBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    const auto findResult = m_meshInstanceTable.find(proxy.nProxyId);
    if (findResult == m_meshInstanceTable.end()) { return; }

    if (proxy.nProxyId == MGlobal::M_INVALID_UINDEX)
    {
        MORTY_ASSERT(false);
        return;
    }

    size_t nIdx = findResult->second;
    m_batchGroup[nIdx]->UpdateMeshInstance(proxy);
}

void MMaterialBatchGroup::UpdateOrCreateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    const auto findResult = m_meshInstanceTable.find(proxy.nProxyId);
    if (findResult == m_meshInstanceTable.end())
    {
        AddMeshInstance(proxy);
        return;
    }

    UpdateMeshInstance(proxy);
}

bool                              MMaterialBatchGroup::IsEmpty() const { return m_meshInstanceTable.empty(); }

MMaterialBatchUtil::TransformType MMaterialBatchUtil::GetMaterialBatchTransformType(MMaterial* pMaterial)
{
    if (!pMaterial) { return TransformType::ENone; }

    if (pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM))
    {
        return TransformType::EUniformArray;
    }
    if (pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE))
    {
        return TransformType::EStorageBuffer;
    }

    return TransformType::ENone;
}

MInstanceBatchGroup* MMaterialBatchUtil::CreateBatchGroup(MMaterial* pMaterial)
{
    auto type = GetMaterialBatchTransformType(pMaterial);

    if (TransformType::EUniformArray == type) { return new MUniformBatchGroup(); }
    if (TransformType::EStorageBuffer == type) { return new MStorageBatchGroup(); }
    if (TransformType::ENone == type) { return new MNoneBatchGroup(); }

    MORTY_ASSERT(false);
    return nullptr;
}
