#include "MShadowMapShaderPropertyBlock.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Shadow/MShadowMeshManager.h"
#include "Resource/MMaterialResource.h"

#include "Material/MMaterial.h"
#include "Material/MComputeDispatcher.h"
#include "Render/MVertex.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"

void MShadowMapShaderPropertyBlock::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;
	MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mps");
	m_pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pMaterial->SetCullMode(MECullMode::ECullNone);
	m_pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, "1");
	m_pMaterial->LoadVertexShader(vs);
	m_pMaterial->LoadPixelShader(ps);

	BindMaterial(m_pMaterial);
}

void MShadowMapShaderPropertyBlock::Release(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pMaterial = nullptr;
}

void MShadowMapShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MORTY_ASSERT(m_pShaderPropertyBlock = pMaterial->GetFramePropertyBlock()->Clone());
	MORTY_ASSERT(m_pWorldMatrixParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneMatrix"));
}

void MShadowMapShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info) 
{
	MViewport* pViewport = info.pViewport;
	MORTY_ASSERT(pViewport);
	MEntity* pCameraEntity = info.pCameraEntity;
	MORTY_ASSERT(pCameraEntity);
	MEntity* pDirectionalEntity = info.pDirectionalLightEntity;
	MORTY_ASSERT(pDirectionalEntity);

	MScene* pScene = pViewport->GetScene();
	const MRenderSystem* pRenderSystem = pScene->GetEngine()->FindSystem<MRenderSystem>();
	auto* pShadowMapManager = pScene->GetManager<MShadowMeshManager>();
	MORTY_ASSERT(pShadowMapManager);

	if (m_pWorldMatrixParam)
	{
		MVariantStruct& cStruct = m_pWorldMatrixParam->var.GetValue<MVariantStruct>();

		MVariantArray& cCamProjArray = cStruct.GetVariant<MVariantArray>("u_matCamProj");


		for (size_t nCascadedIdx = 0; nCascadedIdx < info.shadowRenderInfo.size(); ++nCascadedIdx)
		{
			cCamProjArray[nCascadedIdx].SetValue(info.shadowRenderInfo[nCascadedIdx].m4DirLightInvProj);
		}

		m_pWorldMatrixParam->SetDirty();
	}

}

std::shared_ptr<MShaderPropertyBlock> MShadowMapShaderPropertyBlock::GetPropertyBlock() const
{
	return m_pShaderPropertyBlock;
}