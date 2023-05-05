#include "MShadowMapManager.h"

#include "Scene/MScene.h"
#include "MRenderNotify.h"
#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "Module/MCoreNotify.h"
#include "Render/MVertex.h"
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

	m_tWaitUpdateTransformComponent.erase(pMeshComponent);
	m_tWaitUpdateMeshComponent.erase(pMeshComponent);
	m_tWaitUpdateGenerateShadowComponent.erase(pMeshComponent);

	if (pMeshComponent->GetGenerateDirLightShadow())
	{
		RemoveComponentFromCache(pMeshComponent);
	}
}

void MShadowMapManager::SceneTick(MScene* pScene, const float& fDelta)
{
	for (auto pComponent : m_tWaitUpdateMeshComponent)
	{
		UpdateComponentMesh(pComponent);
	}
	m_tWaitUpdateMeshComponent.clear();

	for (auto pComponent : m_tWaitUpdateTransformComponent)
	{
		UpdateBoundsInWorld(pComponent);
	}
	m_tWaitUpdateTransformComponent.clear();

	for (auto pComponent : m_tWaitUpdateGenerateShadowComponent)
	{
		UpdateGenerateShadow(pComponent);
	}
	m_tWaitUpdateGenerateShadowComponent.clear();

}

void MShadowMapManager::OnTransformChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderableMeshComponent>())
	{
		AddQueueUpdateTransform(pMeshComponent);
	}
}

void MShadowMapManager::OnMeshChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		AddQueueUpdateMesh(pMeshComponent);
	}
}

void MShadowMapManager::OnGenerateShadowChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		AddQueueUpdateGenerateShadow(pMeshComponent);
	}
}

void MShadowMapManager::AddQueueUpdateTransform(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateTransformComponent.insert(pComponent);
}

void MShadowMapManager::AddQueueUpdateMesh(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateMeshComponent.insert(pComponent);
}

void MShadowMapManager::AddQueueUpdateGenerateShadow(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateGenerateShadowComponent.insert(pComponent);
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

void MShadowMapManager::AddComponentToCache(MRenderableMeshComponent* pComponent)
{
	m_staticShadowGroup.AddMeshInstance(pComponent);
}

void MShadowMapManager::RemoveComponentFromCache(MRenderableMeshComponent* pComponent)
{
	m_staticShadowGroup.RemoveMeshInstance(pComponent);
}

void MShadowMapManager::UpdateBoundsInWorld(MRenderableMeshComponent* pComponent)
{
	m_staticShadowGroup.UpdateTransform(pComponent);
}

void MShadowMapManager::UpdateComponentMesh(MRenderableMeshComponent* pComponent)
{
	m_staticShadowGroup.UpdateMesh(pComponent);
}

void MShadowMapManager::UpdateGenerateShadow(MRenderableMeshComponent* pComponent)
{
    MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if(!pSceneComponent)
	{
		MORTY_ASSERT(pSceneComponent);
		return;
	}

	const bool bGenerateShadow = pComponent->GetGenerateDirLightShadow() && pSceneComponent->GetVisibleRecursively();

	if (bGenerateShadow)
	{
		m_staticShadowGroup.AddMeshInstance(pComponent);
	}
	else
	{
		m_staticShadowGroup.RemoveMeshInstance(pComponent);
	}
}
