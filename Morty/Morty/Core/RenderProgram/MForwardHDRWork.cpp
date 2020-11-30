#include "MForwardHDRWork.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardHDRWork, MStandardPostProcessWork)

MForwardHDRWork::MForwardHDRWork()
    : MStandardPostProcessWork()
	, m_pHDRMaterial(nullptr)
{
}

MForwardHDRWork::~MForwardHDRWork()
{
}

void MForwardHDRWork::Render(MPostProcessRenderInfo& info)
{
	info.pRenderer->BeginRenderPass(m_pTempRenderPass, m_pTempRenderTarget);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pHDRMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = info.pPrevLevel->GetBackTexture(info.unFrameIndex)->at(0);
		pMaterialParamSet->m_vTextures[0]->SetDirty();


		Vector2 v2Size = info.pPrevLevel->GetSize();
		pMaterialParamSet->m_vParams[0]->var = v2Size;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(m_pHDRMaterial))
	{
		info.pRenderer->DrawMesh(m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass();
}

void MForwardHDRWork::Initialize(MIRenderProgram* pRenderProgram)
{
	Super::Initialize(pRenderProgram);

	InitializeMaterial();
}

void MForwardHDRWork::Release()
{
	ReleaseMaterial();

	Super::Release();
}

void MForwardHDRWork::InitializeMaterial()
{
	m_pHDRMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();

	MResource* pVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/high_dynamic_range.mvs");
	MResource* pPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/high_dynamic_range.mps");

	m_pHDRMaterial->LoadVertexShader(pVSResource);
	m_pHDRMaterial->LoadPixelShader(pPSResource);

	m_pHDRMaterial->AddRef();
}

void MForwardHDRWork::ReleaseMaterial()
{
	m_pHDRMaterial->SubRef();
	m_pHDRMaterial = nullptr;
}
