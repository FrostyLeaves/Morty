#include "MCombineWork.h"

#include "MEngine.h"
#include "MMaterial.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MRenderGraph.h"
#include "MForwardPostProcessProgram.h"

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
void MCombineWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardPostProcessProgram* pRenderProgram = dynamic_cast<MForwardPostProcessProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;

	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	MRenderGraphNodeInput* pInput0 = pGraphNode->GetInput(0);
	MRenderGraphNodeInput* pInput1 = pGraphNode->GetInput(1);

	if (!pInput0 || !pInput1)
		return;

	MRenderGraphTexture* pInputTex0 = pInput0->GetLinkedTexture();
	MRenderGraphTexture* pInputTex1 = pInput1->GetLinkedTexture();

	if (!pInputTex0 || !pInputTex1)
		return;

	info.pRenderer->SetRenderToTextureBarrier({ pInputTex0->GetRenderTexture(), pInputTex1->GetRenderTexture() });

	info.pRenderer->BeginRenderPass(pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = pInputTex0->GetRenderTexture();
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		pMaterialParamSet->m_vTextures[1]->pTexture = pInputTex1->GetRenderTexture();
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
