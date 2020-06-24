#include "MShadowTextureRenderTarget.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MIRenderer.h"
#include "MMaterial.h"
#include "MScene.h"
#include "MViewport.h"

#include "Model/MModelInstance.h"
#include "Model/MSkinnedMeshInstance.h"
#include "MSkeleton.h"


M_OBJECT_IMPLEMENT(MShadowTextureRenderTarget, MTextureRenderTarget)

MShadowTextureRenderTarget::MShadowTextureRenderTarget()
	: MTextureRenderTarget()
	, m_m4LightInvProj(Matrix4::IdentityMatrix)
	, m_pShadowRenderGroup(nullptr)
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

void MShadowTextureRenderTarget::Render(MIRenderer* pRenderer, const Matrix4& m4InvProj, std::vector<MShadowRenderGroup>* pGroup)
{
	m_m4LightInvProj = m4InvProj;

	m_pShadowRenderGroup = pGroup;

	pRenderer->Render(this);

	m_pShadowRenderGroup = nullptr;
}

void MShadowTextureRenderTarget::OnCreated()
{

	Super::OnCreated();

	MMaterialResource* pShadowMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW);
	m_pStaticMaterial = pShadowMaterialRes;

	MMaterialResource* pShadowWithAnimMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_ANIM);
	m_pAnimMaterial = pShadowWithAnimMaterialRes;
	
	std::vector<MERenderTextureType> vEmpty;
	Initialize(MTextureRenderTarget::ERenderDepth, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE, vEmpty);
}

void MShadowTextureRenderTarget::OnDelete()
{
	Super::OnDelete();
}

void MShadowTextureRenderTarget::OnRender(MIRenderer* pRenderer)
{
	if (nullptr == m_pShadowRenderGroup)
		return;

	if (nullptr == m_pMeshParam)
		m_pMeshParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == m_pWorldParam)
		m_pWorldParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_WORLD_MATRIX);
	if (nullptr == m_pAnimBonesParam)
		m_pAnimBonesParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);

	pRenderer->SetViewport(0.0f, 0.0f, MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE, 0.0f, 1.0f);

	MStruct& cWorldStruct = *m_pWorldParam->var.GetStruct();
	MStruct& cMeshStruct = *m_pMeshParam->var.GetStruct();
	cWorldStruct[0] = m_m4LightInvProj;
	m_pWorldParam->SetDirty();
	pRenderer->SetVertexShaderParam(*m_pWorldParam);

	for (MShadowRenderGroup& group : *m_pShadowRenderGroup)
	{
		if (MSkeletonInstance* pSkeleton = group.pSkeletonInstance)
		{
			MVariant& cVariant = (*m_pAnimBonesParam->var.GetStruct())[0];
			MVariantArray& cBonesArray = *cVariant.GetArray();

			const std::vector<MBone>& bones = pSkeleton->GetAllBones();
			uint32_t size = bones.size();
			if (size > MBONES_MAX_NUMBER)
				size = MBONES_MAX_NUMBER;
			for (uint32_t i = 0; i < size; ++i)
			{
				cBonesArray[i] = bones[i].m_matWorldTransform;
			}

			m_pAnimBonesParam->SetDirty();
			pRenderer->SetVertexShaderParam(*m_pAnimBonesParam);

			pRenderer->SetUseMaterial(m_pAnimMaterial);
		}
		else
		{
			pRenderer->SetUseMaterial(m_pStaticMaterial);
		}

		for (MIMeshInstance* pMeshIns : group.vMeshInstances)
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