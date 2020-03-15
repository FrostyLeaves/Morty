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
	, m_pStaticMaterial(nullptr)
	, m_pAnimMaterial(nullptr)
	, m_pMeshParam(nullptr)
	, m_pWorldParam(nullptr)
	, m_pAnimBonesParam(nullptr)
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
	
	m_pDevice = m_pEngine->GetDevice();
	m_eRenderTargetType = MTextureRenderTarget::ERenderDepth;

	OnResize(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE);
}

void MShadowTextureRenderTarget::OnRender(MIRenderer* pRenderer)
{
	if (nullptr == m_pScene)
		return;

	if (nullptr == m_pMeshParam)
		m_pMeshParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == m_pWorldParam)
		m_pWorldParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_WORLD_MATRIX);
	if (nullptr == m_pAnimBonesParam)
		m_pAnimBonesParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);

	pRenderer->SetViewport(0.0f, 0.0f, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE, 0.0f, 1.0f);

	MDirectionalLight* pLight = m_pScene->FindActiveDirectionLight();
	if (nullptr == pLight)
		return;
	MViewport* pViewport = GetSourceViewport();
	Matrix4 matLightInvProj = pViewport->GetLightInverseProjection(pLight);

	MStruct& cWorldStruct = *m_pWorldParam->var.GetStruct();
	MStruct& cMeshStruct = *m_pMeshParam->var.GetStruct();
	cWorldStruct[0] = matLightInvProj;
	m_pWorldParam->SetDirty();
	pRenderer->SetVertexShaderParam(*m_pWorldParam);

	for (MModelInstance* pModelIns : *m_pScene->GetModelInstances())
	{
		if (!pModelIns->GetVisibleRecursively())
			continue;

		if (!pModelIns->GetGenerateDirLightShadow())
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
			pRenderer->SetVertexShaderParam(*m_pAnimBonesParam);

			pRenderer->SetUseMaterial(m_pAnimMaterial);
		}
		else
		{
			pRenderer->SetUseMaterial(m_pStaticMaterial);
		}

		for (MNode* pChild : pModelIns->GetFixedChildren())
		{
			if (!pChild->GetVisibleRecursively())
				continue;

			if (MIMeshInstance* pMeshIns = pChild->DynamicCast<MIMeshInstance>())
			{

				Matrix4 worldTrans = pMeshIns->GetWorldTransform();

				cMeshStruct[0] = worldTrans;
				m_pMeshParam->SetDirty();
				pRenderer->SetVertexShaderParam(*m_pMeshParam);

				pRenderer->UpdateMaterialParam();
				pRenderer->DrawMesh(pMeshIns->GetMesh());
			}
		}
	}
}