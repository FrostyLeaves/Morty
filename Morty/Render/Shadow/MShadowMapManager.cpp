#include "MShadowMapManager.h"

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
#include "System/MNotifySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Resource/MMaterialResource.h"


MORTY_CLASS_IMPLEMENT(MShadowMapManager, IManager)

constexpr size_t TransformStructSize = sizeof(MMeshInstanceTransform);

void MShadowMapManager::Initialize()
{
	Super::Initialize();
	InitializeMaterial();


	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::OnTransformChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::OnMeshChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::OnGenerateShadowChanged, this));
	}

	if (m_pUpdateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>("RenderMeshManagerUpdate"))
	{
		m_pUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::RenderUpdate, this));
		m_pUpdateTask->SetThreadType(METhreadType::ERenderThread);
	}
}

void MShadowMapManager::Release()
{
	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::OnTransformChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::OnMeshChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MShadowMapManager::OnGenerateShadowChanged, this));
	}

	ReleaseMaterial();
	Super::Release();
}

std::set<const MType*> MShadowMapManager::RegisterComponentType() const
{
	return { MRenderableMeshComponent::GetClassType() };
}

void MShadowMapManager::RegisterComponent(MComponent* pComponent)
{
	if (!pComponent)
	{
		return;
	}

	const auto pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>();
	if (!pMeshComponent)
	{
		return;
	}

	auto pMaterial = pMeshComponent->GetMaterial();
	if (!pMaterial)
	{
		return;
	}

	if (!IsGenerateShadowMaterial(pMaterial->GetMaterialType()))
	{
		return;
	}

	m_tWaitAddComponent[pMeshComponent] = MRenderableMaterialGroup::CreateProxyFromComponent(pMeshComponent);
}

void MShadowMapManager::UnregisterComponent(MComponent* pComponent)
{
	if(!pComponent)
	{
		return;
	}

	const auto pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>();
	if (!pMeshComponent)
	{
		return;
	}

	m_tWaitUpdateComponent.erase(pMeshComponent);
	m_tWaitRemoveComponent.insert(pMeshComponent);
}

void MShadowMapManager::RenderUpdate(MTaskNode* pNode)
{
	if (!m_tWaitRemoveComponent.empty())
	{
		for (auto& key : m_tWaitRemoveComponent)
		{
			m_staticShadowGroup.RemoveMeshInstance(key);
		}
		m_tWaitRemoveComponent.clear();
	}

	if (!m_tWaitAddComponent.empty())
	{
		for (auto& [key, proxy] : m_tWaitUpdateComponent)
		{
			m_staticShadowGroup.AddMeshInstance(key, proxy);
		}
		m_tWaitAddComponent.clear();
	}

	if (!m_tWaitUpdateComponent.empty())
	{
		for (auto& [key, proxy] : m_tWaitUpdateComponent)
		{
			m_staticShadowGroup.UpdateMeshInstance(key, proxy);
		}
		m_tWaitUpdateComponent.clear();
	}
}

void MShadowMapManager::OnTransformChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderableMeshComponent>())
	{
		AddToUpdateQueue(pMeshComponent);
	}
}

void MShadowMapManager::OnMeshChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		AddToUpdateQueue(pMeshComponent);
	}
}

void MShadowMapManager::OnGenerateShadowChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		AddToUpdateQueue(pMeshComponent);
	}
}

bool MShadowMapManager::IsGenerateShadowMaterial(MEMaterialType eType) const
{
	return eType == MEMaterialType::EDefault
        || eType == MEMaterialType::EDeferred;
}

void MShadowMapManager::AddToUpdateQueue(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateComponent[pComponent] = MRenderableMaterialGroup::CreateProxyFromComponent(pComponent);
}

void MShadowMapManager::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mps");
	auto pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->SetRasterizerType(MERasterizerType::ECullNone);
	pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, "true");
	pMaterial->LoadVertexShader(vs);
	pMaterial->LoadPixelShader(ps);

	m_staticShadowGroup.Initialize(GetEngine(), pMaterial);
}

void MShadowMapManager::ReleaseMaterial()
{
	m_staticShadowGroup.Release(GetEngine());
}

void MShadowMapManager::RemoveComponentFromCache(MRenderableMeshComponent* pComponent)
{
	m_staticShadowGroup.RemoveMeshInstance(pComponent);
}
