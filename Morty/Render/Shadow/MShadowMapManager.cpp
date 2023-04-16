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
constexpr size_t CullingStructSize = sizeof(MMergeInstanceCullData);

void MShadowMapManager::Initialize()
{
	Super::Initialize();
	InitializeBuffer();
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
	ReleaseBuffer();
	Super::Release();
}

std::set<const MType*> MShadowMapManager::RegisterComponentType() const
{
	return { MRenderableMeshComponent::GetClassType() };
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
	m_tWaitUploadCullingBufferComponent.erase(pMeshComponent);

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
		m_tWaitUploadCullingBufferComponent.insert(pComponent);
	}
	m_tWaitUpdateMeshComponent.clear();

	for (auto pComponent : m_tWaitUpdateTransformComponent)
	{
		UpdateBoundsInWorld(pComponent);
		m_tWaitUploadCullingBufferComponent.insert(pComponent);
	}
	m_tWaitUpdateTransformComponent.clear();

	for (auto pComponent : m_tWaitUpdateGenerateShadowComponent)
	{
		UpdateGenerateShadow(pComponent);
	}
	m_tWaitUpdateGenerateShadowComponent.clear();

	for (auto pComponent : m_tWaitUploadCullingBufferComponent)
	{
	    UploadCullingBuffer(pComponent);
	}
	m_tWaitUploadCullingBufferComponent.clear();
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
}

void MShadowMapManager::ReleaseMaterial()
{

}

void MShadowMapManager::InitializeBuffer()
{

	m_transformBuffer.buffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_transformBuffer.buffer.m_eUsageType = MBuffer::MUsageType::EStorage;

	m_cullingBuffer.buffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_cullingBuffer.buffer.m_eUsageType = MBuffer::MUsageType::EStorage;


#if MORTY_DEBUG
	m_transformBuffer.buffer.m_strDebugBufferName = "Shadow Instance Transform Buffer";
	m_cullingBuffer.buffer.m_strDebugBufferName = "Shadow Culling Buffer";
#endif

}

void MShadowMapManager::ReleaseBuffer()
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_transformBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_cullingBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MShadowMapManager::AddComponentToCache(MRenderableMeshComponent* pComponent)
{
	if (!m_tShadowInstanceCache.HasItem(pComponent))
	{
		const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
		const size_t nIdx = m_tShadowInstanceCache.AddItem(pComponent, {});
		const size_t nItemNum = m_tShadowInstanceCache.GetItems().size();
		const size_t nTransformBufferSize = nItemNum * TransformStructSize;
		const size_t nCullingBufferSize = nItemNum * CullingStructSize;
		if (m_transformBuffer.GetSize() < nTransformBufferSize)
		{
			m_transformBuffer.ResizeMemory(pRenderSystem->GetDevice(), nTransformBufferSize);
		}
		if (m_cullingBuffer.GetSize() < nCullingBufferSize)
		{
			m_cullingBuffer.ResizeMemory(pRenderSystem->GetDevice(), nCullingBufferSize);
		}

		if (MShadowMeshInstance* pInstance = m_tShadowInstanceCache.FindItem(pComponent))
		{
			pInstance->transformBuffer.begin = (nIdx * TransformStructSize);
			pInstance->transformBuffer.size = TransformStructSize;

			pInstance->cullingBuffer.begin = (nIdx * CullingStructSize);
			pInstance->cullingBuffer.size = CullingStructSize;
		}
	}

	UpdateComponentMesh(pComponent);
	UpdateBoundsInWorld(pComponent);
}

void MShadowMapManager::RemoveComponentFromCache(MRenderableMeshComponent* pComponent)
{
	m_tShadowInstanceCache.RemoveItem(pComponent);
}

void MShadowMapManager::UpdateBoundsInWorld(MRenderableMeshComponent* pComponent)
{
	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();

	MShadowMeshInstance* pInstance = m_tShadowInstanceCache.FindItem(pComponent);
	if (!pInstance)
	{
		return;
	}

	UpdateInstanceBounds(pInstance, pSceneComponent->GetWorldPosition(), pSceneComponent->GetWorldScale());
	UploadTransformBuffer(pComponent);
}

void MShadowMapManager::UpdateComponentMesh(MRenderableMeshComponent* pComponent)
{
	MShadowMeshInstance* pInstance = m_tShadowInstanceCache.FindItem(pComponent);
	if (!pInstance)
	{
		return;
	}

	MResourceRef meshResource = pComponent->GetMeshResource();
	auto pMeshResource = meshResource.GetResource<MMeshResource>();
	if (!pMeshResource)
	{
		return;
	}

	pInstance->pMesh = pMeshResource->GetMesh();
	pInstance->boundsSphere = *pMeshResource->GetMeshesDefaultSphere();
}

void MShadowMapManager::UpdateGenerateShadow(MRenderableMeshComponent* pComponent)
{
	bool bGenerateShadow = pComponent->GetGenerateDirLightShadow();
	if (bGenerateShadow)
	{
		if (m_tShadowInstanceCache.FindItem(pComponent))
		{
			return;
		}
		else
		{
			AddComponentToCache(pComponent);
			m_tWaitUploadCullingBufferComponent.insert(pComponent);
		}
	}
	else
	{
		RemoveComponentFromCache(pComponent);
		m_tWaitUploadCullingBufferComponent.erase(pComponent);
		m_tWaitUpdateTransformComponent.erase(pComponent);
	    m_tWaitUpdateMeshComponent.erase(pComponent);
	}
}

void MShadowMapManager::UpdateInstanceBounds(MShadowMeshInstance* pInstance, Vector3 worldPosition, Vector3 worldScale)
{
	const Vector3 scale = worldScale;
	const float regularScale = (std::max)((std::max)(scale.x, scale.y), scale.z);
	pInstance->boundsInWorld.m_v3CenterPoint = worldPosition;
	pInstance->boundsInWorld.m_fRadius = pInstance->boundsSphere.m_fRadius * regularScale;
}

void MShadowMapManager::UploadCullingBuffer(MRenderableMeshComponent* pComponent)
{
	if (MShadowMeshInstance* pInstance = m_tShadowInstanceCache.FindItem(pComponent))
	{
		UploadCullingBuffer(pInstance);
	}
}

void MShadowMapManager::UploadCullingBuffer(MShadowMeshInstance* pInstance)
{
	MMeshManager* pMeshManager = GetScene()->GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager->HasMesh(pInstance->pMesh))
	{
		pMeshManager->RegisterMesh(pInstance->pMesh);
	}
	auto meshData = pMeshManager->FindMesh(pInstance->pMesh);

	MMergeInstanceCullData cullData;
	cullData.position = pInstance->boundsInWorld.m_v3CenterPoint;
	cullData.radius = pInstance->boundsInWorld.m_fRadius;
	for (size_t nLodIdx = 0; nLodIdx < MRenderGlobal::MESH_LOD_LEVEL_RANGE; ++nLodIdx)
	{
		//TODO LOD
		cullData.lods[nLodIdx].distance = 500 * (1 + nLodIdx) / MRenderGlobal::MESH_LOD_LEVEL_RANGE;
		cullData.lods[nLodIdx].firstIndex = meshData.indexInfo.begin;
		cullData.lods[nLodIdx].indexCount = meshData.indexInfo.size;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	const auto& cullingBuffer = pInstance->cullingBuffer;
	
	m_cullingBuffer.UploadBuffer(pRenderSystem->GetDevice(), cullingBuffer.begin, reinterpret_cast<const MByte*>(&cullData), cullingBuffer.size);

}

void MShadowMapManager::UploadTransformBuffer(MRenderableMeshComponent* pComponent)
{
	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		MORTY_ASSERT(pSceneComponent);
		return;
	}
	
	MShadowMeshInstance* pInstance = m_tShadowInstanceCache.FindItem(pComponent);
	if (!pInstance)
	{
		MORTY_ASSERT(pInstance);
		return;
	}

	const auto& transformBuffer = pInstance->transformBuffer;
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	MMeshInstanceTransform data;
	data.transform = pSceneComponent->GetWorldTransform();
	data.normalTransform = Matrix3(data.transform, 3, 3);
	m_transformBuffer.UploadBuffer(pRenderSystem->GetDevice(), transformBuffer.begin, reinterpret_cast<const MByte*>(&data), transformBuffer.size);
}

size_t MShadowMapManager::GetInstanceNum() const
{
	const size_t nItemNum = m_tShadowInstanceCache.GetItems().size();
	return nItemNum;
}
