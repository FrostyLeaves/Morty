#include "MMaterialBatchGroup.h"

#include <utility>

#include "BatchGroup/MStorageBatchGroup.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Mesh/MVertex.h"
#include "Scene/MEntity.h"
#include "System/MRenderSystem.h"

using namespace morty;

class MORTY_API MMaterialBatchUtil
{
public:
    static MInstanceBatchGroup* CreateBatchGroup(MMaterialTemplate* pMaterial);
};

void MMaterialBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterialTemplate> pMaterial)
{
    m_engine           = pEngine;
    m_materialTemplate = std::move(pMaterial);
    m_batchGroup.Initialize(pEngine, m_materialTemplate);
}

void                     MMaterialBatchGroup::Release(MEngine* pEngine) { m_batchGroup.Release(pEngine); }

MMeshInstanceRenderProxy MMaterialBatchGroup::CreateProxyFromComponent(MRenderMeshComponent* pComponent)
{
    MMeshInstanceRenderProxy proxy;
    proxy.bVisible    = true;
    proxy.bCullEnable = pComponent->GetSceneCullEnable();
    proxy.nProxyId    = static_cast<uint32_t>(pComponent->GetComponentID().nIdx);
    proxy.nSkeletonId = static_cast<uint32_t>(pComponent->GetAttachedModelComponentID().nIdx);
    if (auto* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>())
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
    size_t nIndexInGroup = m_batchGroup.AddMeshInstance(proxy);
    MORTY_UNUSED(nIndexInGroup);
}

void MMaterialBatchGroup::RemoveMeshInstance(MMeshInstanceKey nProxyId) { m_batchGroup.RemoveMeshInstance(nProxyId); }

void MMaterialBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    m_batchGroup.UpdateMeshInstance(proxy);
}

void MMaterialBatchGroup::UpdateOrCreateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
    if (m_batchGroup.HasMeshInstance(proxy))
    {
        UpdateMeshInstance(proxy);
        return;
    }

    AddMeshInstance(proxy);
}

bool                 MMaterialBatchGroup::IsEmpty() const { return m_batchGroup.IsEmpty(); }

MInstanceBatchGroup* MMaterialBatchUtil::CreateBatchGroup(MMaterialTemplate* pMaterial)
{
    MORTY_ASSERT((pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE)));
    return new MStorageBatchGroup();
}
