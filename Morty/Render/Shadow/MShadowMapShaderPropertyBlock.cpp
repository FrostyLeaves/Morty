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
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mps");
	m_pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pMaterial->SetCullMode(MECullMode::ECullNone);
	m_pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	m_pMaterial->LoadShader(vs);
	m_pMaterial->LoadShader(ps);

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
	MORTY_ASSERT(m_pShaderPropertyBlock = pMaterial->GetMaterialPropertyBlock()->Clone());
	MORTY_ASSERT(m_pWorldMatrixParam = m_pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::SHADOW_GENERATE_CBUFFER_MATRIX_NAME));
}

void MShadowMapShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info) 
{
	if (m_pWorldMatrixParam)
	{
		MVariantStruct& cStruct = m_pWorldMatrixParam->var.GetValue<MVariantStruct>();

		MVariantArray& cCamProjArray = cStruct.GetVariant<MVariantArray>(MShaderPropertyName::SHADOW_GENERATE_LIGHT_PROJ_MATRIX);

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