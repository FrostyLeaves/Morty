#include "MAnimationManager.h"

#include "Batch/MMeshInstanceManager.h"
#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Utility/MFunction.h"

#include "Resource/MMaterialResource.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MModelComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Model/MAnimationRenderProxy.h"
#include "Model/MSkeletalAnimation.h"
#include "Module/MCoreNotify.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"

#include "RenderProgram/RenderGraph/MRenderCommon.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MAnimationManager, IManager)

void MAnimationManager::Initialize()
{
    Super::Initialize();

    if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        pNotifySystem->RegisterNotify(
                MCoreNotify::NOTIFY_PARENT_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnSceneParentChanged, this)
        );
        pNotifySystem->RegisterNotify(
                MRenderNotify::NOTIFY_ANIMATION_POSE_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnPoseChanged, this)
        );
    }

    m_updateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>(MRenderGlobal::TASK_ANIMATION_MANAGER_UPDATE);
    if (m_updateTask)
    {
        m_updateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::RenderUpdate, this));
        m_updateTask->SetThreadType(METhreadType::ERenderThread);
    }

    m_animationRenderGroup = std::make_shared<MAnimationRenderGroup>();
    m_animationRenderGroup->Initialize(GetEngine());
}

void MAnimationManager::Release()
{
    if (m_updateTask)
    {
        GetEngine()->GetMainGraph()->DestroyNode(m_updateTask);
        m_updateTask = nullptr;
    }

    m_animationRenderGroup->Release(GetEngine());
    m_animationRenderGroup = nullptr;

    if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
    {
        pNotifySystem->UnregisterNotify(
                MCoreNotify::NOTIFY_PARENT_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnSceneParentChanged, this)
        );
        pNotifySystem->UnregisterNotify(
                MRenderNotify::NOTIFY_ANIMATION_POSE_CHANGED,
                M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnPoseChanged, this)
        );
    }

    Super::Release();
}

std::set<const MType*> MAnimationManager::RegisterComponentType() const { return {MModelComponent::GetClassType()}; }

void                   MAnimationManager::UnregisterComponent(MComponent* pComponent)
{
    if (MModelComponent* pModelComponent = pComponent->template DynamicCast<MModelComponent>())
    {
        RemoveComponent(pModelComponent);
    }
}

void MAnimationManager::OnSceneParentChanged(MComponent* pComponent)
{
    auto pSceneComponent = pComponent->template DynamicCast<MSceneComponent>();
    if (!pSceneComponent)
    {
        MORTY_ASSERT(pSceneComponent);
        return;
    }

    auto pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderMeshComponent>();
    if (!pMeshComponent) { return; }

    MModelComponent*   pModelComponent = FindAttachedModelComponent(pSceneComponent);
    const MComponentID pModelIdx       = pModelComponent ? pModelComponent->GetComponentID() : MComponentID();

    pMeshComponent->SetAttachedModelComponentID(pModelIdx);
}

void MAnimationManager::OnPoseChanged(MComponent* pComponent)
{
    auto pModelComponent = pComponent->template DynamicCast<MModelComponent>();
    if (!pModelComponent)
    {
        MORTY_ASSERT(pModelComponent);
        return;
    }

    auto pSkeleton = GetSkeletonInstance(pModelComponent);
    if (!pSkeleton)
    {
        MORTY_ASSERT(pSkeleton);
        return;
    }

    const auto                 renderPose           = MAnimationRenderGroup::CreatePoseProxy(pSkeleton);
    const MSkeletonInstanceKey nSkeletonInstanceIdx = pModelComponent->GetComponentID().nIdx;

    m_waitUpdateComponent[nSkeletonInstanceIdx] = renderPose;
}

void MAnimationManager::RemoveComponent(MModelComponent* pModelComponent)
{
    const MSkeletonInstanceKey nSkeletonInstanceIdx = pModelComponent->GetComponentID().nIdx;

    m_waitUpdateComponent.erase(nSkeletonInstanceIdx);
    m_waitRemoveComponent.insert(nSkeletonInstanceIdx);
}

void MAnimationManager::SceneTick(MScene* pScene, const float& fDelta)
{
    MComponentGroup<MModelComponent>* pModelComponents = pScene->FindComponents<MModelComponent>();
    if (!pModelComponents) { return; }

    for (MModelComponent& modelComponent: pModelComponents->m_components)
    {
        if (!modelComponent.IsValid()) { continue; }

        auto pController = modelComponent.GetSkeletalAnimationController();
        if (!pController) { continue; }

        if (pController->GetState() != MIAnimController::EPlay) { continue; }

        bool bVisible = false;
        if (MSceneComponent* pSceneComponent = modelComponent.GetEntity()->GetComponent<MSceneComponent>())
        {
            bVisible = pSceneComponent->GetVisibleRecursively();
        }

        pController->NextStep(fDelta, bVisible);
        modelComponent.SendComponentNotify(MRenderNotify::NOTIFY_ANIMATION_POSE_CHANGED);
    }
}

void MAnimationManager::RenderUpdate(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    for (auto& nProxyIdx: m_waitRemoveComponent) { m_animationRenderGroup->RemoveSkeletonRenderInstance(nProxyIdx); }
    m_waitRemoveComponent.clear();

    for (auto& [nProxyIdx, poseProxy]: m_waitUpdateComponent)
    {
        m_animationRenderGroup->UpdateOrCreateMeshInstance(nProxyIdx, poseProxy);
    }
    m_waitUpdateComponent.clear();
}

MAnimationBufferData MAnimationManager::GetAnimationBuffer() const
{
    return m_animationRenderGroup->GetAnimationBuffer();
}

std::shared_ptr<IPropertyBlockAdapter> MAnimationManager::CreateAnimationPropertyAdapter()
{
    return m_animationRenderGroup;
}

MModelComponent* MAnimationManager::FindAttachedModelComponent(MSceneComponent* pSceneComponent)
{
    while (pSceneComponent)
    {
        MEntity* pEntity = pSceneComponent->GetEntity();
        if (MModelComponent* pModelComponent = pEntity->GetComponent<MModelComponent>()) { return pModelComponent; }

        pSceneComponent = GetScene()
                                  ->GetComponent(pSceneComponent->GetParentComponent())
                                  ->template DynamicCast<MSceneComponent>();
    }

    return nullptr;
}

MSkeletonInstance* MAnimationManager::GetSkeletonInstance(MModelComponent* pCompnoent)
{
    return pCompnoent->GetSkeleton();
}
