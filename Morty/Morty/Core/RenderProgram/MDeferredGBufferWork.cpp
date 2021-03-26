#include "MDeferredGBufferWork.h"

#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

M_OBJECT_IMPLEMENT(MDeferredGBufferWork, MObject)

MDeferredGBufferWork::MDeferredGBufferWork()
    : MObject()
    , m_pRenderProgram(nullptr)
{
}

MDeferredGBufferWork::~MDeferredGBufferWork()
{
}

void MDeferredGBufferWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeShaderParamSet();
	InitializeRenderGraph();
}

void MDeferredGBufferWork::Release()
{
	ReleaseRenderGraph();
	ReleaseShaderParamSet();
}

void MDeferredGBufferWork::InitializeShaderParamSet()
{

}

void MDeferredGBufferWork::ReleaseShaderParamSet()
{

}

void MDeferredGBufferWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->FindRenderGraphTexture("Output Target");
	if (nullptr == pOutputTargetTexture)
	{
		pOutputTargetTexture = pRenderGraph->AddRenderGraphTexture("Output Target");
		pOutputTargetTexture->SetUsage(METextureUsage::ERenderBack);
		pOutputTargetTexture->SetLayout(METextureLayout::ERGBA8);
		pOutputTargetTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pOutputTargetTexture->SetSize(Vector2(1.0f, 1.0f));
	}

	MRenderGraphTexture* pOutputDepthTexture = pRenderGraph->FindRenderGraphTexture("Output Depth");
	if (nullptr == pOutputDepthTexture)
	{
		pOutputDepthTexture = pRenderGraph->AddRenderGraphTexture("Output Depth");
		pOutputDepthTexture->SetUsage(METextureUsage::ERenderDepth);
		pOutputDepthTexture->SetLayout(METextureLayout::EDepth);
		pOutputDepthTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pOutputDepthTexture->SetSize(Vector2(1.0f, 1.0f));
	}

	MRenderGraphNode* pForwardNode = pRenderGraph->AddRenderGraphNode("Forward Node");

	MRenderGraphNodeInput* pInputNode = pForwardNode->AppendInput();
	MRenderGraphNodeOutput* pOutputTarget = pForwardNode->AppendOutput();
	MRenderGraphNodeOutput* pOutputDepth = pForwardNode->AppendOutput();

	pOutputTarget->SetClear(true);
	pOutputTarget->SetClearColor(m_pRenderProgram->GetClearColor());
	pOutputTarget->SetRenderTexture(pOutputTargetTexture);
	pRenderGraph->SetFinalOutput(pOutputTarget);

	pOutputDepth->SetClear(true);
	pOutputDepth->SetRenderTexture(pOutputDepthTexture);



	pForwardNode->BindRenderFunction(std::bind(&MDeferredGBufferWork::Render, this, std::placeholders::_1));
}

void MDeferredGBufferWork::ReleaseRenderGraph()
{

}

void MDeferredGBufferWork::OnDelete()
{

}

void MDeferredGBufferWork::Render(MRenderGraphNode* pGraphNode)
{

}

void MDeferredGBufferWork::UpdateShaderSharedParams(MRenderInfo& info, MForwardRenderShaderParamSet& frameParamSet)
{

}
