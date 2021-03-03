#include "MCombineWork.h"

#include "MEngine.h"
#include "MMaterial.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

#include "MForwardPostProcessProgram.h"

M_OBJECT_IMPLEMENT(MCombineWork, MIPostProcessWork)

MCombineWork::MCombineWork()
	: MIPostProcessWork()
	, m_strGraphNodeName("")
	, m_pRenderProgram(nullptr)
	, m_aMaterial()
	, m_pScreenDrawMesh(nullptr)
{

}

MCombineWork::~MCombineWork()
{

}

void MCombineWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;
	Super::Initialize(pRenderProgram);

	InitializeMesh();
	InitializeMaterial();
	InitializeGraph();
}

void MCombineWork::Release()
{
	ReleaseGraph();
	ReleaseMaterial();
	ReleaseMesh();

	Super::Release();
	m_pRenderProgram = nullptr;
}
void MCombineWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardPostProcessProgram* pRenderProgram = dynamic_cast<MForwardPostProcessProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;

	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	MRenderGraphNodeOutput* pOutput = pGraphNode->GetOutput(0);
	if (!pOutput)
		return;

	MRenderGraphTexture* pOutputTexture = pOutput->GetRenderTexture();
	if (!pOutputTexture)
		return;

	MRenderGraphNodeInput* pInput0 = pGraphNode->GetInput(0);
	MRenderGraphNodeInput* pInput1 = pGraphNode->GetInput(1);
	if (!pInput0 || !pInput1)
		return;

	MRenderGraphTexture* pInputTex0 = pInput0->GetLinkedTexture();
	MRenderGraphTexture* pInputTex1 = pInput1->GetLinkedTexture();
	if (!pInputTex0 || !pInputTex1)
		return;


	info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, { pInputTex0->GetRenderTexture(), pInputTex1->GetRenderTexture() });

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);


	Vector2 v2OutputSize = pOutputTexture->GetOutputSize();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));

	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[info.unFrameIndex]->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = pInputTex0->GetRenderTexture();
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		pMaterialParamSet->m_vTextures[1]->pTexture = pInputTex1->GetRenderTexture();
		pMaterialParamSet->m_vTextures[1]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_aMaterial[info.unFrameIndex]))
	{
		info.pRenderer->DrawMesh(info.pPrimaryCommand, m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MCombineWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MCombineWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MCombineWork::InitializeGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MStandardPostProcessWork::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MRenderGraphTexture* pTempOutputTexture = pRenderGraph->AddRenderGraphTexture("Combine_Post");
	if (pTempOutputTexture)
	{
		pTempOutputTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pTempOutputTexture->SetSize(Vector2(1.0f, 1.0f));
		pTempOutputTexture->SetUsage(pOutputTargetTexture->GetUsage());
	}

	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode("Combine_Post"))
	{
		m_strGraphNodeName = pPostProcessNode->GetNodeName();

		pPostProcessNode->AppendInput();
		pPostProcessNode->AppendInput();

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(pTempOutputTexture);
		}

		pPostProcessNode->BindRenderFunction(std::bind(&MCombineWork::Render, this, std::placeholders::_1));
	}
}

void MCombineWork::ReleaseGraph()
{

}

void MCombineWork::InitializeMaterial()
{
	for(uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_aMaterial[i] = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
		m_aMaterial[i]->LoadVertexShader("./Shader/post_process_basic.mvs");
		m_aMaterial[i]->LoadPixelShader("./Shader/post_process_combine.mps");

		m_aMaterial[i]->AddRef();
	}
}

void MCombineWork::ReleaseMaterial()
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (m_aMaterial[i])
		{
			m_aMaterial[i]->SubRef();
			m_aMaterial[i] = nullptr;
		}
	}
}
