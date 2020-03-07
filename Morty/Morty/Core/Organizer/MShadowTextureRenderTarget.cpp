#include "MShadowTextureRenderTarget.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"

#include "MIRenderer.h"
#include "MMaterial.h"
#include "MScene.h"
#include "MViewport.h"

#include "MModelInstance.h"
#include "MSkeleton.h"
#include "MSkinnedMeshInstance.h"


MTypeIdentifierImplement(MShadowTextureRenderTarget, MObject)

MShadowTextureRenderTarget::MShadowTextureRenderTarget()
	: MTextureRenderTarget()
	, m_pScene(nullptr)
	, m_pStaticMeshParam(nullptr)
	, m_pAnimMeshParam(nullptr)
	, m_pStaticWorldParam(nullptr)
	, m_pAnimWorldParam(nullptr)
	, m_pAnimBonesParam(nullptr)
	, m_pStaticMaterial(nullptr)
	, m_pAnimMaterial(nullptr)
{

}

MShadowTextureRenderTarget::~MShadowTextureRenderTarget()
{

}

void MShadowTextureRenderTarget::OnCreated()
{
	MMaterialResource* pShadowMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW);
	m_pStaticMaterial = pShadowMaterialRes->GetMaterialTemplate();

	MMaterialResource* pShadowWithAnimMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_ANIM);
	m_pAnimMaterial = pShadowWithAnimMaterialRes->GetMaterialTemplate();

	{
		std::vector<MShaderParam>& vVtxParams = m_pStaticMaterial->GetVertexShaderParams();
		for (MShaderParam& param : vVtxParams)
		{
			if (param.unCode == SHADER_PARAM_CODE_MESH_MATRIX)
			{
				m_pStaticMeshParam = &param;
				continue;
			}
			else if (param.unCode == SHADER_PARAM_CODE_WORLD_MATRIX)
			{
				m_pStaticWorldParam = &param;
				continue;
			}
		}
	}

	{
		std::vector<MShaderParam>& vVtxParams = m_pAnimMaterial->GetVertexShaderParams();
		for (MShaderParam& param : vVtxParams)
		{
			if (param.unCode == SHADER_PARAM_CODE_MESH_MATRIX)
			{
				m_pAnimMeshParam = &param;
				continue;
			}
			else if (param.unCode == SHADER_PARAM_CODE_WORLD_MATRIX)
			{
				m_pAnimWorldParam = &param;
				continue;
			}
			else if (param.unCode == SHADER_PARAM_CODE_ANIMATION)
			{
				m_pAnimBonesParam = &param;
				continue;
			}
		}
	}

	m_pDevice = m_pEngine->GetDevice();
	m_eRenderTargetType = MTextureRenderTarget::ERenderDepth;

	OnResize(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE);
}

void MShadowTextureRenderTarget::OnRender(MIRenderer* pRenderer)
{
	if (nullptr == m_pScene)
		return;

	pRenderer->SetViewport(0.0f, 0.0f, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE, 0.0f, 1.0f);

	MDirectionalLight* pLight = m_pScene->FindActiveDirectionLight();
	MViewport* pViewport = GetSourceViewport();
	Matrix4 matLightInvProj = pViewport->GetLightInverseProjection(pLight);

	{
		pRenderer->SetUseMaterial(m_pStaticMaterial);

		MStruct& cWorldStruct = *m_pStaticWorldParam->var.GetStruct();
		MStruct& cMeshStruct = *m_pStaticMeshParam->var.GetStruct();
		cWorldStruct[0] = matLightInvProj;
		m_pStaticWorldParam->SetDirty();

		for (MModelInstance* pModelIns : *m_pScene->GetStaticModels())
		{
			if (!pModelIns->GetVisibleRecursively())
				continue;

			for (MNode* pChild : pModelIns->GetFixedChildren())
			{
				if (!pChild->GetVisibleRecursively())
					continue;

				if (MIMeshInstance* pMeshIns = pChild->DynamicCast<MIMeshInstance>())
				{
					Matrix4 worldTrans = pMeshIns->GetWorldTransform();

					cMeshStruct[0] = worldTrans;
					m_pStaticMeshParam->SetDirty();

					pRenderer->UpdateMaterialParam();
					pRenderer->DrawMesh(pMeshIns->GetMesh());
				}
			}
		}
	}


	{
		pRenderer->SetUseMaterial(m_pAnimMaterial);

		MStruct& cWorldStruct = *m_pAnimWorldParam->var.GetStruct();
		MStruct& cMeshStruct = *m_pAnimMeshParam->var.GetStruct();
		cWorldStruct[0] = matLightInvProj;
		m_pAnimWorldParam->SetDirty();

		for (MModelInstance* pModelIns : *m_pScene->GetAnimationalModels())
		{
			if (!pModelIns->GetVisibleRecursively())
				continue;

			if (MSkeletonInstance* pSkeleton = pModelIns->GetSkeleton())
			{
				
				MVariant& cVariant = (*m_pAnimBonesParam->var.GetStruct())[0];
				MVariantArray& cBonesArray = *cVariant.GetArray();

				const std::vector<MBone*>& bones = pSkeleton->GetAllBones();
				unsigned int size = bones.size();
				if (size > MBONES_MAX_NUMBER)
					size = MBONES_MAX_NUMBER;
				for (unsigned int i = 0; i < size; ++i)
				{
					cBonesArray[i] = bones[i]->GetTransformInModelWorld();
				}

				m_pAnimBonesParam->SetDirty();
			}

			for (MNode* pChild : pModelIns->GetFixedChildren())
			{
				if (!pChild->GetVisibleRecursively())
					continue;

				if (MIMeshInstance* pMeshIns = pChild->DynamicCast<MIMeshInstance>())
				{
					Matrix4 worldTrans = pMeshIns->GetWorldTransform();

					cMeshStruct[0] = worldTrans;
					m_pAnimMeshParam->SetDirty();

					pRenderer->UpdateMaterialParam();
					pRenderer->DrawMesh(pMeshIns->GetMesh());
				}
			}
		}
	}
}