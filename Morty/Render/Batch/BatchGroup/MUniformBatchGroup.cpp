#include "MUniformBatchGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Render/MVertex.h"
#include "Utility/MGlobal.h"


void MUniformBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
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
		m_pTransformParam = m_pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_MESH_MATRIX);
	}

	MStruct& meshMatrixCbuffer = m_pTransformParam->var.GetValue<MStruct>();
	MVariantArray& arr = meshMatrixCbuffer.GetVariant<MVariantArray>(MShaderPropertyName::MESH_LOCAL_MATRIX);
	m_nMaxInstanceNum = arr.MemberNum();

	m_vTransformArray.resize(m_nMaxInstanceNum);
	for (size_t nIdx = 0; nIdx < m_nMaxInstanceNum; ++nIdx)
	{
		MVariantStruct& srt = arr[nIdx].GetValue<MVariantStruct>();
		m_vTransformArray[nIdx].worldMatrix =srt.FindVariant(MShaderPropertyName::MESH_WORLD_MATRIX);
		m_vTransformArray[nIdx].normalMatrix = srt.FindVariant(MShaderPropertyName::MESH_NORMAL_MATRIX);
		m_vTransformArray[nIdx].instanceIndex = srt.FindVariant(MShaderPropertyName::MESH_INSTANCE_INDEX);
	}
}

void MUniformBatchGroup::Release(MEngine* pEngine)
{
	const MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pTransformParam = nullptr;
	m_pMaterial = nullptr;
    m_nCurrentInstanceNum = 0;
    m_nMaxInstanceNum = 1;
	
	m_tInstanceCache = {};
}

bool MUniformBatchGroup::CanAddMeshInstance() const
{
	return m_nCurrentInstanceNum < m_nMaxInstanceNum;
}

void MUniformBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	auto key = proxy.nProxyId;

	if(key == MGlobal::M_INVALID_UINDEX)
	{
		MORTY_ASSERT(key);
		return;
	}

	if (!m_pTransformParam)
	{
		MORTY_ASSERT(m_pTransformParam);
		return;
	}

	if (m_nCurrentInstanceNum >= m_nMaxInstanceNum)
	{
		MORTY_ASSERT(m_nCurrentInstanceNum < m_nMaxInstanceNum);
		return;
	}

	if (m_tInstanceCache.HasItem(key))
	{
		MORTY_ASSERT(false);
		return;
	}

	m_tInstanceCache.AddItem(key, {});
	++m_nCurrentInstanceNum;
	

	UpdateMeshInstance(proxy);
}

void MUniformBatchGroup::RemoveMeshInstance(MMeshInstanceKey key)
{
	const auto pInstance = m_tInstanceCache.FindItem(key);
	if (nullptr == pInstance)
	{
		MORTY_ASSERT(false);
		return;
	}
	
	m_tInstanceCache.RemoveItem(key);
	--m_nCurrentInstanceNum;
}

void MUniformBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	auto key = proxy.nProxyId;
	const auto pInstance = m_tInstanceCache.FindItem(key);
	if (nullptr == pInstance)
	{
		return;
	}
	
	size_t nIdx = m_tInstanceCache.GetItemIdx(key);

	//Transposed and Inverse.
	Matrix3 matNormal(proxy.worldTransform, 3, 3);
	m_vTransformArray[nIdx].normalMatrix.SetValue(matNormal);
	m_vTransformArray[nIdx].worldMatrix.SetValue(proxy.worldTransform);
	m_vTransformArray[nIdx].instanceIndex.SetValue(Vector4(proxy.nProxyId, proxy.nSkeletonId, 0, 0));
	m_pTransformParam->SetDirty();
	
	*pInstance = proxy;
}

MMeshInstanceRenderProxy* MUniformBatchGroup::FindMeshInstance(MMeshInstanceKey key)
{
	auto result = m_tInstanceCache.FindItem(key);
	return result;
}

void MUniformBatchGroup::InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func)
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
