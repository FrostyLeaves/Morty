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

void MNoneBatchGroup::AddMeshInstance(MRenderableMeshComponent* pComponent)
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

	if (m_bInstanceValid)
	{
		MORTY_ASSERT(!m_bInstanceValid);
		return;
	}

	MResourceRef meshResource = pComponent->GetMeshResource();
	auto pMeshResource = meshResource.GetResource<MMeshResource>();
	if (!pMeshResource)
	{
		return;
	}

	m_instance.pMesh = pMeshResource->GetMesh();
	m_instance.bounds = *pMeshResource->GetMeshesDefaultOBB();
	m_instance.bVisible = true;

	UpdateTransform(pComponent);

	m_bInstanceValid = true;
}

void MNoneBatchGroup::RemoveMeshInstance(MRenderableMeshComponent* pComponent)
{
	m_bInstanceValid = false;
}

void MNoneBatchGroup::UpdateTransform(MRenderableMeshComponent* pComponent)
{
	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		MORTY_ASSERT(pSceneComponent);
		return;
	}
	
	Matrix4 worldTrans = pSceneComponent->GetWorldTransform();

	if (m_worldMatrix.IsValid())
	{
		m_worldMatrix.SetValue(worldTrans);
	}

	if (m_normalMatrix.IsValid())
	{
		//Transposed and Inverse.
		Matrix3 matNormal(worldTrans, 3, 3);

		m_normalMatrix.SetValue(matNormal);
	}

	m_pTransformParam->SetDirty();


	const Matrix4 matWorldTrans = pSceneComponent->GetWorldTransform();
	const Vector3 v3Position = pSceneComponent->GetWorldPosition();
	m_instance.boundsWithTransform.SetBoundsOBB(v3Position, matWorldTrans, m_instance.bounds);
}

void MNoneBatchGroup::InstanceExecute(std::function<void(const MRenderableMeshInstance&, size_t nIdx)> func)
{
	func(m_instance, 0);
}
