#include "MStorageBatchGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderParamSet.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Render/MVertex.h"

constexpr size_t TransformStructSize = sizeof(MMeshInstanceTransform);

void MStorageBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
{
	m_pEngine = pEngine;
	m_pMaterial = pMaterial;
	if (m_pShaderPropertyBlock)
	{
		const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
		m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
		m_pShaderPropertyBlock = nullptr;
		m_pTransformParam = nullptr;
	}

	if (!pMaterial)
	{
		MORTY_ASSERT(pMaterial);
		return;
	}

	if (std::shared_ptr<MShaderPropertyBlock> pTemplatePropertyBlock = pMaterial->GetMeshParamSet())
	{
		m_pShaderPropertyBlock = pTemplatePropertyBlock->Clone();
		m_pTransformParam = m_pShaderPropertyBlock->FindStorageParam("u_meshMatrix");
	}

	m_transformBuffer.buffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
	m_transformBuffer.buffer.m_eUsageType = MBuffer::MUsageType::EStorage;
#if MORTY_DEBUG
	m_transformBuffer.buffer.m_strDebugBufferName = "Storage Batch Instance Transform Buffer";
#endif

	m_pTransformParam->pBuffer = &m_transformBuffer.buffer;
}

void MStorageBatchGroup::Release(MEngine* pEngine)
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pTransformParam = nullptr;
	m_pMaterial = nullptr;
	m_tInstanceCache = {};


	m_transformBuffer.buffer.DestroyBuffer(pRenderSystem->GetDevice());
}

bool MStorageBatchGroup::CanAddMeshInstance() const
{
	return true;
}

void MStorageBatchGroup::AddMeshInstance(MRenderableMeshComponent* pComponent)
{
	if(!pComponent)
	{
		MORTY_ASSERT(pComponent);
		return;
	}

	if (!m_pTransformParam)
	{
		MORTY_ASSERT(m_pTransformParam);
		return;
	}

	if (m_tInstanceCache.HasItem(pComponent))
	{
		MORTY_ASSERT(false);
		return;
	}

	MResourceRef meshResource = pComponent->GetMeshResource();
	auto pMeshResource = meshResource.GetResource<MMeshResource>();
	if (!pMeshResource)
	{
		return;
	}

	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	const size_t nCurrentIdx = m_tInstanceCache.AddItem(pComponent, {});
	const size_t nItemNum = m_tInstanceCache.GetItems().size();
	const size_t nTransformBufferSize = nItemNum * TransformStructSize;
	if (m_transformBuffer.GetSize() < nTransformBufferSize)
	{
		m_transformBuffer.ResizeMemory(pRenderSystem->GetDevice(), nTransformBufferSize);
	}

	if (m_vTransformArray.size() < nItemNum)
	{
		m_vTransformArray.resize(nItemNum);
	}

	MRenderableMeshInstance& instance = *m_tInstanceCache.FindItem(pComponent);
	instance.bVisible = true;
	instance.pMesh = pMeshResource->GetMesh();
	instance.bounds = *pMeshResource->GetMeshesDefaultOBB();
	m_vTransformArray[nCurrentIdx].begin = nCurrentIdx * TransformStructSize;
	m_vTransformArray[nCurrentIdx].size = TransformStructSize;
	
	UpdateTransform(pComponent);
}

void MStorageBatchGroup::RemoveMeshInstance(MRenderableMeshComponent* pComponent)
{
	const auto pInstance = m_tInstanceCache.FindItem(pComponent);
	if (nullptr == pInstance)
	{
		MORTY_ASSERT(false);
		return;
	}
	
	m_tInstanceCache.RemoveItem(pComponent);
}

void MStorageBatchGroup::UpdateTransform(MRenderableMeshComponent* pComponent)
{
	const auto pInstance = m_tInstanceCache.FindItem(pComponent);
	if (nullptr == pInstance)
	{
		return;
	}

	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		MORTY_ASSERT(pSceneComponent);
		return;
	}

	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	const size_t nCurrentIdx = m_tInstanceCache.GetItemIdx(pComponent);
	const auto& transformMemory = m_vTransformArray[nCurrentIdx];

	MMeshInstanceTransform data;
	data.transform = pSceneComponent->GetWorldTransform();
	data.normalTransform = Matrix3(data.transform, 3, 3);
	m_transformBuffer.UploadBuffer(pRenderSystem->GetDevice(), transformMemory.begin, reinterpret_cast<const MByte*>(&data), transformMemory.size);

	const Vector3 v3Position = pSceneComponent->GetWorldPosition();
	pInstance->boundsWithTransform.SetBoundsOBB(v3Position, data.transform, pInstance->bounds);
}

MRenderableMeshInstance* MStorageBatchGroup::FindMeshInstance(MRenderableMeshComponent* pComponent)
{
	auto result = m_tInstanceCache.FindItem(pComponent);
	return result;
}

void MStorageBatchGroup::InstanceExecute(std::function<void(const MRenderableMeshInstance&, size_t nIdx)> func)
{
	const auto& items = m_tInstanceCache.GetItems();
	for (size_t nIdx = 0; nIdx < items.size(); ++nIdx)
	{
		if (items[nIdx].bValid)
		{
			func(items[nIdx].value, nIdx);
		}
	}
}
