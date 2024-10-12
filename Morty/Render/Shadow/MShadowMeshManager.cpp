#include "MShadowMeshManager.h"

#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Material/MComputeDispatcher.h"
#include "Mesh/MVertex.h"
#include "Module/MCoreNotify.h"
#include "Scene/MScene.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"


#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MGlobal.h"
#include "Resource/MMaterialResource.h"
#include "Utility/MMaterialName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShadowMeshManager, IManager)

void MShadowMeshManager::Initialize()
{
    Super::Initialize();
    InitializeMaterial();


    if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        pNotifySystem->RegisterNotify(
                MCoreNotify::NOTIFY_TRANSFORM_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnTransformChanged, this)
        );
        pNotifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_MESH_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnMeshChanged, this)
        );
        pNotifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnGenerateShadowChanged, this)
        );
    }

    m_updateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>(MRenderGlobal::TASK_SHADOWMAP_MANAGER_UPDATE);
    if (m_updateTask)
    {
        m_updateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::RenderUpdate, this));
        m_updateTask->SetThreadType(METhreadType::ERenderThread);
    }
}

void MShadowMeshManager::Release()
{
    if (m_updateTask)
    {
        GetEngine()->GetMainGraph()->DestroyNode(m_updateTask);
        m_updateTask = nullptr;
    }

    if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        pNotifySystem->UnregisterNotify(
                MCoreNotify::NOTIFY_TRANSFORM_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnTransformChanged, this)
        );
        pNotifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_MESH_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnMeshChanged, this)
        );
        pNotifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnGenerateShadowChanged, this)
        );
    }

    ReleaseMaterial();
    Super::Release();
}

std::set<const MType*> MShadowMeshManager::RegisterComponentType() const
{
    return {MRenderMeshComponent::GetClassType()};
}

void MShadowMeshManager::UnregisterComponent(MComponent* pComponent)
{
    if (!pComponent) { return; }

    const auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>();
    if (!pMeshComponent) { return; }

    AddToDeleteQueue(pMeshComponent);
}

void MShadowMeshManager::RenderUpdate(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    for (auto& [type, group]: m_batchMaterialGroup)
    {
        if (!group->tWaitRemoveComponent.empty())
        {
            for (auto& key: group->tWaitRemoveComponent) { group->materialGroup.RemoveMeshInstance(key); }
            group->tWaitRemoveComponent.clear();
        }

        if (!group->tWaitUpdateComponent.empty())
        {
            for (auto& [key, proxy]: group->tWaitUpdateComponent)
            {
                group->materialGroup.UpdateOrCreateMeshInstance(proxy);
            }
            group->tWaitUpdateComponent.clear();
        }
    }
}

void MShadowMeshManager::OnTransformChanged(MComponent* pComponent)
{
    if (auto pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderMeshComponent>())
    {
        if (pMeshComponent->GetGenerateDirLightShadow()) { AddToUpdateQueue(pMeshComponent); }
    }
}

void MShadowMeshManager::OnMeshChanged(MComponent* pComponent)
{
    if (auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>())
    {
        if (pMeshComponent->GetGenerateDirLightShadow()) { AddToUpdateQueue(pMeshComponent); }
    }
}

void MShadowMeshManager::OnGenerateShadowChanged(MComponent* pComponent)
{
    if (auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>())
    {
        if (pMeshComponent->GetGenerateDirLightShadow()) { AddToUpdateQueue(pMeshComponent); }
        else { AddToDeleteQueue(pMeshComponent); }
    }
}

void MShadowMeshManager::AddToUpdateQueue(MRenderMeshComponent* pComponent)
{
    auto pMeshResource = pComponent->GetMeshResource().GetResource<MMeshResource>();
    if (!pMeshResource) { return; }

    auto eVertexType = pMeshResource->GetMeshVertexType();
    auto findGroup   = m_batchMaterialGroup.find(eVertexType);
    if (findGroup == m_batchMaterialGroup.end())
    {
        MORTY_ASSERT(findGroup != m_batchMaterialGroup.end());
        return;
    }

    auto pMaterialGroup = findGroup->second;

    auto proxy = MMaterialBatchGroup::CreateProxyFromComponent(pComponent);

    if (m_componentTable.find(pComponent) != m_componentTable.end())
    {
        if (m_componentTable[pComponent] != pMaterialGroup) { AddToDeleteQueue(pComponent); }
    }

    pMaterialGroup->tWaitUpdateComponent[proxy.nProxyId] = proxy;
    m_componentTable[pComponent]                         = pMaterialGroup;
}

void MShadowMeshManager::AddToDeleteQueue(MRenderMeshComponent* pComponent)
{
    auto nProxyId = pComponent->GetComponentID().nIdx;

    auto findResult = m_componentTable.find(pComponent);
    if (findResult == m_componentTable.end()) { return; }

    MaterialGroup* pGroup = findResult->second;
    pGroup->tWaitUpdateComponent.erase(nProxyId);
    pGroup->tWaitRemoveComponent.insert(nProxyId);
    m_componentTable.erase(findResult);
}

void MShadowMeshManager::InitializeMaterial()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    auto             pTemplate       = pResourceSystem->LoadResource(MMaterialName::SHADOW_MAP);
    auto             pMaterial       = MMaterial::CreateMaterial(pTemplate);
    m_staticMaterial.SetResource(pMaterial);
    m_batchMaterialGroup[MEMeshVertexType::Normal] = new MaterialGroup();
    m_batchMaterialGroup[MEMeshVertexType::Normal]->materialGroup.Initialize(GetEngine(), pMaterial);

    pTemplate = pResourceSystem->LoadResource(MMaterialName::SHADOW_MAP_SKELETON);
    pMaterial = MMaterial::CreateMaterial(pTemplate);
    m_animatedMaterial.SetResource(pMaterial);
    m_batchMaterialGroup[MEMeshVertexType::Skeleton] = new MaterialGroup();
    m_batchMaterialGroup[MEMeshVertexType::Skeleton]->materialGroup.Initialize(GetEngine(), pMaterial);


    m_materialGroup.push_back(&m_batchMaterialGroup[MEMeshVertexType::Normal]->materialGroup);
    m_materialGroup.push_back(&m_batchMaterialGroup[MEMeshVertexType::Skeleton]->materialGroup);
}

void MShadowMeshManager::ReleaseMaterial()
{
    for (auto& [type, group]: m_batchMaterialGroup)
    {
        group->materialGroup.Release(GetEngine());
        delete group;
        group = nullptr;
    }
    m_batchMaterialGroup.clear();
    m_componentTable.clear();
    m_staticMaterial.SetResource(nullptr);
    m_animatedMaterial.SetResource(nullptr);
}

std::vector<MMaterialBatchGroup*> MShadowMeshManager::GetAllShadowGroup() const { return m_materialGroup; }
