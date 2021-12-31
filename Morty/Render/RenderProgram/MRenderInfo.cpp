#include "MRenderInfo.h"


#include "MScene.h"
#include "MViewport.h"
#include "MMaterial.h"

#include "MSceneComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

MRenderInfo::MRenderInfo()
	: nFrameIndex(0)
	, fDelta(0.0f)
	, fGameTime(0.0f)
	, pViewport(nullptr)
	, pCameraEntity(nullptr)
	, pDirectionalLightEntity(nullptr)
	, pShadowMapTexture(nullptr)
	, pFrameShaderParamSet(nullptr)
	, m4DirLightInvProj()
	, cGenerateShadowRenderAABB()
	, cCaclShadowRenderAABB()
	, pPrimaryRenderCommand(nullptr)
{

}

void MRenderInfo::CollectRenderMesh()
{
	MScene* pScene = pViewport->GetScene();

	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	MComponentGroup<MRenderableMeshComponent>* pMeshComponents = pScene->FindComponents<MRenderableMeshComponent>();

	if (!pMeshComponents)
		return;

	for (MRenderableMeshComponent& meshComp : pMeshComponents->m_vComponent)
	{
		MMaterial* pMaterial = meshComp.GetMaterial();
		if (!pMaterial)
			continue;

		MSceneComponent* pSceneComponent = meshComp.GetEntity()->GetComponent<MSceneComponent>();

		if (!pSceneComponent->GetVisibleRecursively())
			continue;

		const MBoundsAABB* pBounds = meshComp.GetBoundsAABB();

		if (MCameraFrustum::EOUTSIDE == pViewport->GetCameraFrustum().ContainTest(*pBounds))
			continue;

		if (pMaterial->GetMaterialType() == MEMaterialType::EDepthPeel)
		{
			auto& meshes = m_tTransparentGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}
		else
		{
			auto& meshes = m_tMaterialGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}


		if (meshComp.GetShadowType() != MRenderableMeshComponent::MEShadowType::ENone)
		{
			pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
		}
	}

	cCaclShadowRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}

void MRenderInfo::CollectShadowMesh()
{
	if (!pViewport)
		return;

	MScene* pScene = pViewport->GetScene();
	if (!pScene)
		return;

	if (!pDirectionalLightEntity)
	{
		pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();

		if (!pDirectionalLightEntity)
			return;
	}

	MSceneComponent* pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
	if (!pLightSceneComponent)
		return;

	MComponentGroup<MRenderableMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderableMeshComponent>();
	if (!pMeshComponentGroup)
		return;

	Vector3 v3LightDir = pLightSceneComponent->GetForward();

	bool bGenerateShadow = false;
	Vector3 v3ShadowMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3ShadowMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::map<MSkeletonInstance*, size_t> tSkeletonToIndex;


	for (MRenderableMeshComponent& component : pMeshComponentGroup->m_vComponent)
	{
		if (!component.IsValid())
			continue;

		if (MMaterial* pMaterial = component.GetMaterial())
		{
			if (pMaterial->GetMaterialType() != MEMaterialType::EDefault)
				continue;
		}

		MEntity* pEntity = component.GetEntity();

		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

		if (pSceneComponent->GetVisibleRecursively() && component.GetGenerateDirLightShadow())
		{
			MSkeletonInstance* pSkeletonInstance = component.GetSkeletonInstance();

			auto& vMeshes = m_tShadowGroupMesh[pSkeletonInstance];


			if (const MBoundsAABB* pBounds = component.GetBoundsAABB())
			{
				if (pViewport->GetCameraFrustum().ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
				{
					vMeshes.push_back(&component);
					pBounds->UnionMinMax(v3ShadowMin, v3ShadowMax);

					bGenerateShadow = true;
				}
			}
		}
	}

	if (bGenerateShadow)
	{
		cGenerateShadowRenderAABB.SetMinMax(v3ShadowMin, v3ShadowMax);
	}
	else
	{
		cGenerateShadowRenderAABB = cCaclShadowRenderAABB;
	}

	m4DirLightInvProj = pViewport->GetLightInverseProjection(pDirectionalLightEntity,cCaclShadowRenderAABB, cGenerateShadowRenderAABB);
}
