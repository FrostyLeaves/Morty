#include "MShadowMapShaderParamSet.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "System/MRenderSystem.h"

MShadowMapShaderPropertyBlock::MShadowMapShaderPropertyBlock()
	: m_pShaderPropertyBlock(nullptr)
	, m_pWorldMatrixParam(nullptr)
{
	
}

MShadowMapShaderPropertyBlock::~MShadowMapShaderPropertyBlock()
{
}

void MShadowMapShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MORTY_ASSERT(m_pShaderPropertyBlock = pMaterial->GetFrameParamSet()->Clone());

	MORTY_ASSERT(m_pWorldMatrixParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneMatrix"));
}

void MShadowMapShaderPropertyBlock::ReleaseShaderParamSet(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
	m_pWorldMatrixParam = nullptr;
}

void MShadowMapShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info) const
{
	if (m_pWorldMatrixParam)
	{
		MStruct& cStruct = *m_pWorldMatrixParam->var.GetStruct();

		if (MVariantArray* pCamProjArray = cStruct.GetMember<MVariantArray>(0))
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < info.cCascadedShadow.size(); ++nCascadedIdx)
			{
				(*pCamProjArray)[nCascadedIdx] = info.cCascadedShadow[nCascadedIdx].m4DirLightInvProj;
			}
		}

		m_pWorldMatrixParam->SetDirty();
	}
}
