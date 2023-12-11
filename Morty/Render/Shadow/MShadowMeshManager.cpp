#include "MShadowMeshManager.h"

#include "Scene/MScene.h"
#include "MRenderNotify.h"
#include "Render/MVertex.h"
#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "Module/MCoreNotify.h"
#include "TaskGraph/MTaskGraph.h"
#include "Mesh/MMeshManager.h"
#include "Material/MComputeDispatcher.h"


#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MNotifyManager.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Resource/MMaterialResource.h"
#include "Utility/MGlobal.h"


MORTY_CLASS_IMPLEMENT(MShadowMeshManager, IManager)

void MShadowMeshManager::Initialize()
{
	Super::Initialize();
	InitializeMaterial();


	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnTransformChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnMeshChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnGenerateShadowChanged, this));
	}

	m_pUpdateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>(MRenderGlobal::TASK_SHADOWMAP_MANAGER_UPDATE);
	if (m_pUpdateTask)
	{
		m_pUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::RenderUpdate, this));
		m_pUpdateTask->SetThreadType(METhreadType::ERenderThread);
	}
}

void MShadowMeshManager::Release()
{
	if (m_pUpdateTask)
	{
		GetEngine()->GetMainGraph()->DestroyNode(m_pUpdateTask);
		m_pUpdateTask = nullptr;
	}

	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnTransformChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnMeshChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMeshManager::OnGenerateShadowChanged, this));
	}

	ReleaseMaterial();
	Super::Release();
}

std::set<const MType*> MShadowMeshManager::RegisterComponentType() const
{
	return { MRenderMeshComponent::GetClassType() };
}

void MShadowMeshManager::UnregisterComponent(MComponent* pComponent)
{
	if(!pComponent)
	{
		return;
	}

	const auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>();
	if (!pMeshComponent)
	{
		return;
	}

	AddToDeleteQueue(pMeshComponent);
}

void MShadowMeshManager::RenderUpdate(MTaskNode* pNode)
{
	MORTY_UNUSED(pNode);
	
	for (auto& [type, group] : m_tBatchMaterialGroup)
	{
		if (!group->tWaitRemoveComponent.empty())
		{
			for (auto& key : group->tWaitRemoveComponent)
			{
				group->materialGroup.RemoveMeshInstance(key);
			}
			group->tWaitRemoveComponent.clear();
		}

		if (!group->tWaitUpdateComponent.empty())
		{
			for (auto& [key, proxy] : group->tWaitUpdateComponent)
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
		if (pMeshComponent->GetGenerateDirLightShadow())
		{
			AddToUpdateQueue(pMeshComponent);
		}
	}
}

void MShadowMeshManager::OnMeshChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>())
	{
		if (pMeshComponent->GetGenerateDirLightShadow())
		{
			AddToUpdateQueue(pMeshComponent);
		}
	}
}

void MShadowMeshManager::OnGenerateShadowChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>())
	{
		if (pMeshComponent->GetGenerateDirLightShadow())
		{
			AddToUpdateQueue(pMeshComponent);
		}
        else
        {
			AddToDeleteQueue(pMeshComponent);
        }
	}
}

void MShadowMeshManager::AddToUpdateQueue(MRenderMeshComponent* pComponent)
{
	auto pMeshResource = pComponent->GetMeshResource().GetResource<MMeshResource>();
	if (!pMeshResource)
	{
		return;
	}

	auto eVertexType = pMeshResource->GetMeshVertexType();
	auto findGroup = m_tBatchMaterialGroup.find(eVertexType);
	if (findGroup == m_tBatchMaterialGroup.end())
	{
		MORTY_ASSERT(findGroup != m_tBatchMaterialGroup.end());
		return;
	}

	auto pMaterialGroup = findGroup->second;

	auto proxy = MMaterialBatchGroup::CreateProxyFromComponent(pComponent);

	if (m_tComponentTable.find(pComponent) != m_tComponentTable.end())
	{
		if (m_tComponentTable[pComponent] != pMaterialGroup)
		{
			AddToDeleteQueue(pComponent);
		}
	}

	pMaterialGroup->tWaitUpdateComponent[proxy.nProxyId] = proxy;
	m_tComponentTable[pComponent] = pMaterialGroup;
}

void MShadowMeshManager::AddToDeleteQueue(MRenderMeshComponent* pComponent)
{
	auto nProxyId = pComponent->GetComponentID().nIdx;

	auto findResult = m_tComponentTable.find(pComponent);
	if (findResult == m_tComponentTable.end())
	{
		return;
	}

	MaterialGroup* pGroup = findResult->second;
	pGroup->tWaitUpdateComponent.erase(nProxyId);
	pGroup->tWaitRemoveComponent.insert(nProxyId);
	m_tComponentTable.erase(findResult);
}

void MShadowMeshManager::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mps");
	auto pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->SetCullMode(MECullMode::ECullNone);
	pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	pMaterial->LoadShader(vs);
	pMaterial->LoadShader(ps);
	m_staticMaterial.SetResource(pMaterial);
	m_tBatchMaterialGroup[MEMeshVertexType::Normal] = new MaterialGroup();
	m_tBatchMaterialGroup[MEMeshVertexType::Normal]->materialGroup.Initialize(GetEngine(), pMaterial);

	pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->SetCullMode(MECullMode::ECullNone);
	pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	pMaterial->GetShaderMacro().SetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	pMaterial->LoadShader(vs);
	pMaterial->LoadShader(ps);
	m_animatedMaterial.SetResource(pMaterial);
	m_tBatchMaterialGroup[MEMeshVertexType::Skeleton] = new MaterialGroup();
	m_tBatchMaterialGroup[MEMeshVertexType::Skeleton]->materialGroup.Initialize(GetEngine(), pMaterial);


	m_vMaterialGroup.push_back(&m_tBatchMaterialGroup[MEMeshVertexType::Normal]->materialGroup);
	m_vMaterialGroup.push_back(&m_tBatchMaterialGroup[MEMeshVertexType::Skeleton]->materialGroup);
}

void MShadowMeshManager::ReleaseMaterial()
{
	for (auto& [type, group] : m_tBatchMaterialGroup)
	{
		group->materialGroup.Release(GetEngine());
		delete group;
		group = nullptr;
	}
	m_tBatchMaterialGroup.clear();
	m_tComponentTable.clear();
	m_staticMaterial.SetResource(nullptr);
	m_animatedMaterial.SetResource(nullptr);
}

std::vector<MMaterialBatchGroup*> MShadowMeshManager::GetAllShadowGroup() const
{
	return m_vMaterialGroup;
}
