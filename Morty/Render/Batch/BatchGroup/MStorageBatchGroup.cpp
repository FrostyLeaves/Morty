#include "MStorageBatchGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"
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

	if (std::shared_ptr<MShaderPropertyBlock> pTemplatePropertyBlock = pMaterial->GetMeshPropertyBlock())
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

void MStorageBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	auto key = proxy.nProxyId;
	if(key == MGlobal::M_INVALID_INDEX)
	{
		MORTY_ASSERT(key != MGlobal::M_INVALID_INDEX);
		return;
	}

	if (!m_pTransformParam)
	{
		MORTY_ASSERT(m_pTransformParam);
		return;
	}

	if (m_tInstanceCache.HasItem(key))
	{
		MORTY_ASSERT(false);
		return;
	}
	
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	const size_t nCurrentIdx = m_tInstanceCache.AddItem(key, {});
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

	m_vTransformArray[nCurrentIdx].begin = nCurrentIdx * TransformStructSize;
	m_vTransformArray[nCurrentIdx].size = TransformStructSize;
	
	UpdateMeshInstance(proxy);
}

void MStorageBatchGroup::RemoveMeshInstance(MMeshInstanceKey key)
{
	const auto pInstance = m_tInstanceCache.FindItem(key);
	if (nullptr == pInstance)
	{
		MORTY_ASSERT(false);
		return;
	}
	
	m_tInstanceCache.RemoveItem(key);
}

void MStorageBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	auto key = proxy.nProxyId;
	const auto pInstance = m_tInstanceCache.FindItem(key);
	if (nullptr == pInstance)
	{
		return;
	}

	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	const size_t nCurrentIdx = m_tInstanceCache.GetItemIdx(key);
	const auto& transformMemory = m_vTransformArray[nCurrentIdx];

	MMeshInstanceTransform data;
	data.transform = proxy.worldTransform;
	data.normalTransform = Matrix3(data.transform, 3, 3);
	data.instanceIndex.x = proxy.nProxyId;
	data.instanceIndex.y = proxy.nSkeletonId;
	m_transformBuffer.UploadBuffer(pRenderSystem->GetDevice(), transformMemory.begin, reinterpret_cast<const MByte*>(&data), transformMemory.size);
	
	*pInstance = proxy;
}

MMeshInstanceRenderProxy* MStorageBatchGroup::FindMeshInstance(MMeshInstanceKey key)
{
	auto result = m_tInstanceCache.FindItem(key);
	return result;
}

void MStorageBatchGroup::InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func)
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
