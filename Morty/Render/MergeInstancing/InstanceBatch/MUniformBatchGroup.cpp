#include "MUniformBatchGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderParamSet.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Render/MVertex.h"


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

    if (std::shared_ptr<MShaderPropertyBlock> pTemplatePropertyBlock = pMaterial->GetMeshParamSet())
	{
		m_pShaderPropertyBlock = pTemplatePropertyBlock->Clone();
		m_pTransformParam = m_pShaderPropertyBlock->FindConstantParam("u_meshMatrix");
	}

	MStruct& meshMatrixCbuffer = m_pTransformParam->var.GetValue<MStruct>();
	MVariantArray& arr = meshMatrixCbuffer.GetVariant<MVariantArray>("u_meshMatrix");
	m_nMaxInstanceNum = arr.MemberNum();

	m_vTransformArray.resize(m_nMaxInstanceNum);
	for (size_t nIdx = 0; nIdx < m_nMaxInstanceNum; ++nIdx)
	{
		MVariantStruct& srt = arr[nIdx].GetValue<MVariantStruct>();
		m_vTransformArray[nIdx].worldMatrix =srt.FindVariant("u_matWorld");
		m_vTransformArray[nIdx].normalMatrix = srt.FindVariant("u_matNormal");
	}
}

void MUniformBatchGroup::Release(MEngine* pEngine)
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
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

void MUniformBatchGroup::AddMeshInstance(MRenderableMeshComponent* pComponent)
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

	if (m_nCurrentInstanceNum >= m_nMaxInstanceNum)
	{
		MORTY_ASSERT(m_nCurrentInstanceNum < m_nMaxInstanceNum);
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

	const size_t nCurrentIdx = m_tInstanceCache.AddItem(pComponent, {});

	MRenderableMeshInstance& instance = *m_tInstanceCache.FindItem(pComponent);
	instance.bVisible = true;
	instance.pMesh = pMeshResource->GetMesh();
	instance.bounds = *pMeshResource->GetMeshesDefaultOBB();
	++m_nCurrentInstanceNum;
	

	UpdateTransform(pComponent);
}

void MUniformBatchGroup::RemoveMeshInstance(MRenderableMeshComponent* pComponent)
{
	const auto pInstance = m_tInstanceCache.FindItem(pComponent);
	if (nullptr == pInstance)
	{
		MORTY_ASSERT(false);
		return;
	}
	
	m_tInstanceCache.RemoveItem(pComponent);
	--m_nCurrentInstanceNum;
}

void MUniformBatchGroup::UpdateTransform(MRenderableMeshComponent* pComponent)
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

	size_t nIdx = m_tInstanceCache.GetItemIdx(pComponent);
	Matrix4 matWorldTrans = pSceneComponent->GetWorldTransform();

	//Transposed and Inverse.
	Matrix3 matNormal(matWorldTrans, 3, 3);
	m_vTransformArray[nIdx].normalMatrix.SetValue(matNormal);
	m_vTransformArray[nIdx].worldMatrix.SetValue(matWorldTrans);
	m_pTransformParam->SetDirty();

	const Vector3 v3Position = pSceneComponent->GetWorldPosition();
	pInstance->boundsWithTransform.SetBoundsOBB(v3Position, matWorldTrans, pInstance->bounds);
}

MRenderableMeshInstance* MUniformBatchGroup::FindMeshInstance(MRenderableMeshComponent* pComponent)
{
	auto result = m_tInstanceCache.FindItem(pComponent);
	return result;
}

void MUniformBatchGroup::InstanceExecute(std::function<void(const MRenderableMeshInstance&, size_t nIdx)> func)
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
