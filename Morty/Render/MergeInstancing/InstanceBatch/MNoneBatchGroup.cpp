#include "MNoneBatchGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderParamSet.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Render/MVertex.h"


void MNoneBatchGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
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

	if (pMaterial)
	{
		if (std::shared_ptr<MShaderPropertyBlock> pTemplatePropertyBlock = pMaterial->GetMeshParamSet())
		{
			m_pShaderPropertyBlock = pTemplatePropertyBlock->Clone();
			m_pTransformParam = m_pShaderPropertyBlock->FindConstantParam("u_meshMatrix");
			MVariantStruct& srt = m_pTransformParam->var.GetValue<MVariantStruct>();
			m_worldMatrix = srt.FindVariant("u_matWorld");
			m_normalMatrix = srt.FindVariant("u_matNormal");
		}
	}
}

void MNoneBatchGroup::Release(MEngine* pEngine)
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pTransformParam = nullptr;
	m_pMaterial = nullptr;
}

bool MNoneBatchGroup::CanAddMeshInstance() const
{
	return !m_bInstanceValid;
}

void MNoneBatchGroup::AddMeshInstance(MMeshInstanceKey key, MMeshInstanceRenderProxy proxy)
{
	if(!key)
	{
		MORTY_ASSERT(key);
		return;
	}

	if (!m_pTransformParam)
	{
		MORTY_ASSERT(m_pTransformParam);
		return;
	}

	if (m_bInstanceValid)
	{
		MORTY_ASSERT(!m_bInstanceValid);
		return;
	}
	
	UpdateMeshInstance(key, proxy);

	m_bInstanceValid = true;
}

void MNoneBatchGroup::RemoveMeshInstance(MMeshInstanceKey key)
{
	m_bInstanceValid = false;
}

void MNoneBatchGroup::UpdateMeshInstance(MMeshInstanceKey key, MMeshInstanceRenderProxy proxy)
{
	if (m_worldMatrix.IsValid())
	{
		m_worldMatrix.SetValue(proxy.worldTransform);
	}

	if (m_normalMatrix.IsValid())
	{
		//Transposed and Inverse.
		Matrix3 matNormal(proxy.worldTransform, 3, 3);

		m_normalMatrix.SetValue(matNormal);
	}

	m_pTransformParam->SetDirty();
	m_instance = proxy;
}

void MNoneBatchGroup::InstanceExecute(std::function<void(const MMeshInstanceRenderProxy&, size_t nIdx)> func)
{
	func(m_instance, 0);
}
