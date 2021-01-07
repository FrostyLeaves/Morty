#include "MCombineWork.h"

#include "MEngine.h"
#include "MMaterial.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MCombineWork, MStandardPostProcessWork)

MCombineWork::MCombineWork()
	: MStandardPostProcessWork()
	, m_pMaterial(nullptr)
{

}

MCombineWork::~MCombineWork()
{

}

void MCombineWork::Initialize(MIRenderProgram* pRenderProgram)
{
	Super::Initialize(pRenderProgram);

	InitializeMaterial();
}

void MCombineWork::Release()
{
	ReleaseMaterial();

	Super::Release();
}

void MCombineWork::Render(MPostProcessRenderInfo& info)
{
	info.pRenderer->SetRenderToTextureBarrier({ info.pPrevLevelOutput, info.pPrevLevelOutput1 });

	info.pRenderer->BeginRenderPass(m_pTempRenderPass, m_pTempRenderTarget);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = info.pPrevLevelOutput;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		pMaterialParamSet->m_vTextures[1]->pTexture = info.pPrevLevelOutput1;
		pMaterialParamSet->m_vTextures[1]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(m_pMaterial))
	{
		info.pRenderer->DrawMesh(m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass();
}

void MCombineWork::InitializeMaterial()
{
	if (!m_pMaterial)
	{
		m_pMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
		m_pMaterial->LoadVertexShader("./Shader/post_process_basic.mvs");
		m_pMaterial->LoadPixelShader("./Shader/post_process_combine.mps");

		m_pMaterial->AddRef();
	}
}

void MCombineWork::ReleaseMaterial()
{
	if (m_pMaterial)
	{
		m_pMaterial->SubRef();
		m_pMaterial = nullptr;
	}
}
