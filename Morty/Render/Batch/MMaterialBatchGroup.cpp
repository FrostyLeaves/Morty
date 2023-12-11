#include "MMaterialBatchGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "BatchGroup/MNoneBatchGroup.h"
#include "BatchGroup/MUniformBatchGroup.h"
#include "BatchGroup/MStorageBatchGroup.h"
#include "Render/MVertex.h"

class MORTY_API MMaterialBatchUtil
{
public:
	enum MORTY_API TransformType
	{
		ENone = 0,
		EUniformArray = 1,
		EStorageBuffer = 2
	};


	static TransformType GetMaterialBatchTransformType(MMaterial* pMaterial);

	static MInstanceBatchGroup* CreateBatchGroup(MMaterial* pMaterial);
};


void MMaterialBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
{
	m_pEngine = pEngine;
	m_pMaterial = pMaterial;
}

void MMaterialBatchGroup::Release(MEngine* pEngine)
{
	for(auto pMeshGroup : m_vBatchGroup)
	{
		pMeshGroup->Release(pEngine);
		delete pMeshGroup;
	}
	m_vBatchGroup.clear();
}

MMeshInstanceRenderProxy MMaterialBatchGroup::CreateProxyFromComponent(MRenderMeshComponent* pComponent)
{
	MMeshInstanceRenderProxy proxy;
	proxy.bVisible = true;
	proxy.bCullEnable = pComponent->GetSceneCullEnable();
	proxy.nProxyId = static_cast<uint32_t>(pComponent->GetComponentID().nIdx);
	proxy.nSkeletonId = static_cast<uint32_t>(pComponent->GetAttachedModelComponentID().nIdx);
	if (MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>())
	{
		proxy.worldTransform = pSceneComponent->GetWorldTransform();
	}
	else
	{
		proxy.worldTransform = Matrix4::IdentityMatrix;
	}
	
	if (auto pMeshResource = pComponent->GetMeshResource().GetResource<MMeshResource>())
	{
		proxy.pMesh = pMeshResource->GetMesh();
		proxy.bounds = *pMeshResource->GetMeshesDefaultOBB();
		proxy.boundsWithTransform.SetBoundsOBB(proxy.worldTransform.GetTranslation(), proxy.worldTransform, proxy.bounds);
	}

	return proxy;
}

void MMaterialBatchGroup::AddMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	const auto findResult = m_tMeshInstanceTable.find(proxy.nProxyId);
	if (findResult != m_tMeshInstanceTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nMeshGroupIdx = 0;
	MInstanceBatchGroup* pMeshGroup = nullptr;
	for (size_t nIdx = 0; nIdx < m_vBatchGroup.size(); ++nIdx)
	{
		MInstanceBatchGroup* pCurrentGroup = m_vBatchGroup[nIdx];
		if (pCurrentGroup->CanAddMeshInstance())
		{
			nMeshGroupIdx = nIdx;
			pMeshGroup = pCurrentGroup;
			break;
		}
	}
	if (pMeshGroup == nullptr)
	{
		pMeshGroup = MMaterialBatchUtil::CreateBatchGroup(m_pMaterial.get());
		pMeshGroup->Initialize(m_pEngine, m_pMaterial->GetShaderProgram());
		m_vBatchGroup.push_back(pMeshGroup);
		nMeshGroupIdx = m_vBatchGroup.size() - 1;
	}

	if (!pMeshGroup->CanAddMeshInstance())
	{
		MORTY_ASSERT(false);
		return;
	}

	pMeshGroup->AddMeshInstance(proxy);
	m_tMeshInstanceTable[proxy.nProxyId] = nMeshGroupIdx;

}

void MMaterialBatchGroup::RemoveMeshInstance(MMeshInstanceKey nProxyId)
{
	const auto findResult = m_tMeshInstanceTable.find(nProxyId);
	if(findResult == m_tMeshInstanceTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nIdx = findResult->second;
	m_tMeshInstanceTable.erase(findResult);

	if(nIdx >= m_vBatchGroup.size())
	{
		MORTY_ASSERT(nIdx < m_vBatchGroup.size());
		return;
	}

	m_vBatchGroup[nIdx]->RemoveMeshInstance(nProxyId);
}

void MMaterialBatchGroup::UpdateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	const auto findResult = m_tMeshInstanceTable.find(proxy.nProxyId);
	if (findResult == m_tMeshInstanceTable.end())
	{
		return;
	}

	if (proxy.nProxyId == MGlobal::M_INVALID_UINDEX)
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nIdx = findResult->second;
	m_vBatchGroup[nIdx]->UpdateMeshInstance(proxy);
}

void MMaterialBatchGroup::UpdateOrCreateMeshInstance(const MMeshInstanceRenderProxy& proxy)
{
	const auto findResult = m_tMeshInstanceTable.find(proxy.nProxyId);
	if (findResult == m_tMeshInstanceTable.end())
	{
		AddMeshInstance(proxy);
		return;
	}

	UpdateMeshInstance(proxy);
}

bool MMaterialBatchGroup::IsEmpty() const
{
	return m_tMeshInstanceTable.empty();
}

MMaterialBatchUtil::TransformType MMaterialBatchUtil::GetMaterialBatchTransformType(MMaterial* pMaterial)
{
	if (!pMaterial)
	{
		return TransformType::ENone;
	}

	if (pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM))
	{
		return TransformType::EUniformArray;
	}
	if (pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE))
	{
		return TransformType::EStorageBuffer;
	}

	return TransformType::ENone;
}

MInstanceBatchGroup* MMaterialBatchUtil::CreateBatchGroup(MMaterial* pMaterial)
{
	auto type = GetMaterialBatchTransformType(pMaterial);

	if (TransformType::EUniformArray == type)
	{
		return new MUniformBatchGroup();
	}
	if (TransformType::EStorageBuffer == type)
	{
		return new MStorageBatchGroup();
	}
	if (TransformType::ENone == type)
	{
		return new MNoneBatchGroup();
	}

	MORTY_ASSERT(false);
	return nullptr;
}
