#include "MShadowMapShaderParamSet.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MViewport.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MSpotLightComponent.h"
#include "MPointLightComponent.h"
#include "MDirectionalLightComponent.h"

#include "MRenderSystem.h"

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

	MStruct worldMatrixSrt;
	worldMatrixSrt.AppendMVariant("U_matCamProj", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matLightProj", Matrix4());

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
		cStruct[0] = info.m4DirLightInvProj;
		cStruct[1] = info.m4DirLightInvProj;
		cStruct[2] = info.m4DirLightInvProj;

		m_pWorldMatrixParam->SetDirty();
	}
}
