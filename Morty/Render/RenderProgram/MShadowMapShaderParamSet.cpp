#include "MShadowMapShaderParamSet.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "System/MRenderSystem.h"

MShadowMapShaderParamSet::MShadowMapShaderParamSet()
	: MShaderParamSet(1)
	, m_pWorldMatrixParam(nullptr)
{
	
}

MShadowMapShaderParamSet::~MShadowMapShaderParamSet()
{
}

void MShadowMapShaderParamSet::InitializeShaderParamSet(MEngine* pEngine)
{
	m_pWorldMatrixParam = new MShaderConstantParam();
	m_pWorldMatrixParam->unSet = 1;
	m_pWorldMatrixParam->unBinding = 0;
	m_pWorldMatrixParam->eShaderType = MEShaderParamType::EVertex;

	MStruct worldMatrixSrt;

	MVariantArray matCamProjArray;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		matCamProjArray.AppendMVariant<Matrix4>();
	}
	worldMatrixSrt.AppendMVariant("U_matCamProj", matCamProjArray);
	

	m_pWorldMatrixParam->var = worldMatrixSrt;

	m_vParams.push_back(m_pWorldMatrixParam);
}

void MShadowMapShaderParamSet::ReleaseShaderParamSet(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	DestroyBuffer(pRenderSystem->GetDevice());
}

void MShadowMapShaderParamSet::UpdateShaderSharedParams(MRenderInfo& info)
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
