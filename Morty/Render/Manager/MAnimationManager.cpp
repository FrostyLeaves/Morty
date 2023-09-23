#include "MAnimationManager.h"

#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Batch/MMeshInstanceManager.h"
#include "Utility/MFunction.h"

#include "System/MObjectSystem.h"
#include "System/MNotifyManager.h"
#include "System/MResourceSystem.h"
#include "Resource/MMaterialResource.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Component/MSceneComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Model/MAnimationRenderProxy.h"
#include "Module/MCoreNotify.h"

#include "TaskGraph/MTaskGraph.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

MORTY_CLASS_IMPLEMENT(MAnimationManager, IManager)

void MAnimationManager::Initialize()
{
	Super::Initialize();

	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_PARENT_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnSceneParentChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_ANIMATION_POSE_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnPoseChanged, this));
	}

	m_pUpdateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>("AnimationManagerUpdate");
	if (m_pUpdateTask)
	{
		m_pUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::RenderUpdate, this));
		m_pUpdateTask->SetThreadType(METhreadType::ERenderThread);
	}

	m_pAnimationRenderGroup = std::make_shared<MAnimationRenderGroup>();
	m_pAnimationRenderGroup->Initialize(GetEngine());
}

void MAnimationManager::Release()
{
	if (m_pUpdateTask)
	{
		GetEngine()->GetMainGraph()->DestroyNode(m_pUpdateTask);
		m_pUpdateTask = nullptr;
	}

	m_pAnimationRenderGroup->Release(GetEngine());
	m_pAnimationRenderGroup = nullptr;

	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_PARENT_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnSceneParentChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_ANIMATION_POSE_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MAnimationManager::OnPoseChanged, this));
	}

	Super::Release();
}

std::set<const MType*> MAnimationManager::RegisterComponentType() const
{
	return { MModelComponent::GetClassType() };
}

void MAnimationManager::UnregisterComponent(MComponent* pComponent)
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
	if (!pMeshComponent)
	{
		return;
	}
	
	MModelComponent* pModelComponent = FindAttachedModelComponent(pSceneComponent);
	const MComponentID pModelIdx = pModelComponent ? pModelComponent->GetComponentID() : MComponentID();

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

	const auto renderPose = MAnimationRenderGroup::CreatePoseProxy(pSkeleton);
	const MSkeletonInstanceKey nSkeletonInstanceIdx = pModelComponent->GetComponentID().nIdx;

	m_tWaitUpdateComponent[nSkeletonInstanceIdx] = renderPose;
}

void MAnimationManager::RemoveComponent(MModelComponent* pModelComponent)
{
	const MSkeletonInstanceKey nSkeletonInstanceIdx = pModelComponent->GetComponentID().nIdx;

	m_tWaitUpdateComponent.erase(nSkeletonInstanceIdx);
	m_tWaitRemoveComponent.insert(nSkeletonInstanceIdx);
}

void MAnimationManager::RenderUpdate(MTaskNode* pNode)
{
	MORTY_UNUSED(pNode);
	
	for (auto& nProxyIdx : m_tWaitRemoveComponent)
	{
		m_pAnimationRenderGroup->RemoveSkeletonRenderInstance(nProxyIdx);
	}
	m_tWaitRemoveComponent.clear();

	for (auto& [nProxyIdx, poseProxy] : m_tWaitUpdateComponent)
	{
		m_pAnimationRenderGroup->UpdateOrCreateMeshInstance(nProxyIdx, poseProxy);
	}
	m_tWaitUpdateComponent.clear();
}

MAnimationBufferData MAnimationManager::GetAnimationBuffer() const
{
	return m_pAnimationRenderGroup->GetAnimationBuffer();
}

std::shared_ptr<IPropertyBlockAdapter> MAnimationManager::CreateAnimationPropertyAdapter()
{
	return m_pAnimationRenderGroup;
}

MModelComponent* MAnimationManager::FindAttachedModelComponent(MSceneComponent* pSceneComponent)
{
	while (pSceneComponent)
	{
		MEntity* pEntity = pSceneComponent->GetEntity();
		if (MModelComponent* pModelComponent = pEntity->GetComponent<MModelComponent>())
		{
			return pModelComponent;
		}

		pSceneComponent = GetScene()->GetComponent(pSceneComponent->GetParentComponent())->template DynamicCast<MSceneComponent>();
	}

	return nullptr;
}

MSkeletonInstance* MAnimationManager::GetSkeletonInstance(MModelComponent* pCompnoent)
{
	return pCompnoent->GetSkeleton();
}
