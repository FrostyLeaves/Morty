#include "MRenderableMeshGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderParamSet.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"


void MRenderableMeshGroup::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;
}

void MRenderableMeshGroup::Release(MEngine* pEngine)
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pTransformParam = nullptr;
	m_pMaterial = nullptr;
    m_nCurrentInstanceNum = 0;
    m_nMaxInstanceNum = 1;
	m_instancePool = MRepeatIDPool<size_t>();

	m_tMeshComponentTable.clear();
	m_vInstances.clear();
}

void MRenderableMeshGroup::BindMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	m_pMaterial = pMaterial;

	if (m_pShaderPropertyBlock)
	{
		const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
		m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
		m_pShaderPropertyBlock = nullptr;
		m_pTransformParam = nullptr;
	}

	if (pMaterial)
	{
		if (std::shared_ptr<MShaderPropertyBlock> pTemplatePropertyBlock = pMaterial->GetMeshParamSet())
		{
			m_pShaderPropertyBlock = pTemplatePropertyBlock->Clone();
			m_pTransformParam = m_pShaderPropertyBlock->FindConstantParam("_M_E_cbMeshMatrix");
		}

		m_bBatchInstancing = false;
		m_nMaxInstanceNum = 1;
	}
}

bool MRenderableMeshGroup::canAddMeshInstance() const
{
	return m_nCurrentInstanceNum < m_nMaxInstanceNum;
}

void MRenderableMeshGroup::AddMeshInstance(MRenderableMeshComponent* pComponent)
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

	if (m_tMeshComponentTable.find(pComponent) != m_tMeshComponentTable.end())
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

	const size_t nInstanceIdx = m_instancePool.GetNewID();
	if (nInstanceIdx >= m_vInstances.size())
	{
		m_vInstances.resize(nInstanceIdx + 1);
	}

	m_tMeshComponentTable[pComponent] = nInstanceIdx;
	auto& instance = m_vInstances[nInstanceIdx];
	++m_nCurrentInstanceNum;

	instance.bValid = true;
	instance.pMesh = pMeshResource->GetMesh();
	instance.bounds = *pMeshResource->GetMeshesDefaultOBB();

	MVariantStruct& srt = m_pTransformParam->var.GetValue<MVariantStruct>();

	if (m_bBatchInstancing)
	{
		//TODO
		MORTY_ASSERT(false);
	}
	else
	{
		instance.worldMatrixParam = srt.FindVariant("u_matWorld");
		instance.normalMatrixParam = srt.FindVariant("u_matNormal");
	}

	UpdateTransform(pComponent);
}

void MRenderableMeshGroup::RemoveMeshInstance(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult == m_tMeshComponentTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	const size_t nInstanceIdx = findResult->second;
	auto& instance = m_vInstances[nInstanceIdx];
	instance.bValid = false;

	--m_nCurrentInstanceNum;
	m_instancePool.RecoveryID(nInstanceIdx);
	m_tMeshComponentTable.erase(findResult);
}

void MRenderableMeshGroup::UpdateTransform(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult == m_tMeshComponentTable.end())
	{
		return;
	}

	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		MORTY_ASSERT(pSceneComponent);
		return;
	}

	const size_t nInstanceIdx = findResult->second;
	auto& instance = m_vInstances[nInstanceIdx];

	Matrix4 worldTrans = pSceneComponent->GetWorldTransform();

	if (instance.worldMatrixParam.IsValid())
	{
		instance.worldMatrixParam.SetValue(worldTrans);
	}

	if (instance.normalMatrixParam.IsValid())
	{
		//Transposed and Inverse.
		Matrix3 matNormal(worldTrans, 3, 3);

		instance.normalMatrixParam.SetValue(matNormal);
	}

	m_pTransformParam->SetDirty();


	Matrix4 matWorldTrans = pSceneComponent->GetWorldTransform();
	Vector3 v3Position = pSceneComponent->GetWorldPosition();
	instance.boundsWithTransform.SetBoundsOBB(v3Position, matWorldTrans, instance.bounds);
}

void MRenderableMaterialGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
{
	m_pEngine = pEngine;
	m_pMaterial = pMaterial;
}

void MRenderableMaterialGroup::Release(MEngine* pEngine)
{
	for(auto pMeshGroup : m_vRenderableMeshGroup)
	{
		pMeshGroup->Release(pEngine);
		delete pMeshGroup;
	}
	m_vRenderableMeshGroup.clear();
}

void MRenderableMaterialGroup::AddMeshInstance(MRenderableMeshComponent* pComponent)
{
	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if (!pComponent)
	{
		MORTY_ASSERT(false);
		return;
	}

	if (!pSceneComponent)
	{
		MORTY_ASSERT(false);
		return;
	}

	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult != m_tMeshComponentTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nMeshGroupIdx = 0;
	MRenderableMeshGroup* pMeshGroup = nullptr;
	for (size_t nIdx = 0; nIdx < m_vRenderableMeshGroup.size(); ++nIdx)
	{
		MRenderableMeshGroup* pCurrentGroup = m_vRenderableMeshGroup[nIdx];
		if (pCurrentGroup->canAddMeshInstance())
		{
			nMeshGroupIdx = nIdx;
			pMeshGroup = pCurrentGroup;
			break;
		}
	}
	if (pMeshGroup == nullptr)
	{
		pMeshGroup = new MRenderableMeshGroup();
		pMeshGroup->Initialize(m_pEngine);
		pMeshGroup->BindMaterial(m_pMaterial);
		m_vRenderableMeshGroup.push_back(pMeshGroup);
		nMeshGroupIdx = m_vRenderableMeshGroup.size() - 1;
	}

	if (!pMeshGroup->canAddMeshInstance())
	{
		MORTY_ASSERT(false);
		return;
	}

	pMeshGroup->AddMeshInstance(pComponent);
	m_tMeshComponentTable[pComponent] = nMeshGroupIdx;

}

void MRenderableMaterialGroup::RemoveMeshInstance(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if(findResult == m_tMeshComponentTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nIdx = findResult->second;
	m_tMeshComponentTable.erase(findResult);

	if(nIdx >= m_vRenderableMeshGroup.size())
	{
		MORTY_ASSERT(nIdx < m_vRenderableMeshGroup.size());
		return;
	}

	m_vRenderableMeshGroup[nIdx]->RemoveMeshInstance(pComponent);
}

void MRenderableMaterialGroup::UpdateTransform(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult == m_tMeshComponentTable.end())
	{
		return;
	}

	size_t nIdx = findResult->second;
	m_vRenderableMeshGroup[nIdx]->UpdateTransform(pComponent);
}

bool MRenderableMaterialGroup::IsEmpty() const
{
	return m_tMeshComponentTable.empty();
}
