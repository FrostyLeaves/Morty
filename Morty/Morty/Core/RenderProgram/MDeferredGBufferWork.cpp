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

	const std::vector<MString> aTextureName = {
		"Output Target",
		"Output Position",
		"Output Albedo",
		"Output Normal",
		"Output Metallic",
		"Output Roughness",
		"Output AmbientOcc"
	};

	MRenderGraphNode* pGBufferNode = pRenderGraph->AddRenderGraphNode("GBuffer Node");

	MRenderGraphNodeInput* pInputNode = pGBufferNode->AppendInput();

	for (const MString& strTextureName : aTextureName)
	{
		MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->FindRenderGraphTexture(strTextureName);
		if (nullptr == pOutputTargetTexture)
		{
			pOutputTargetTexture = pRenderGraph->AddRenderGraphTexture(strTextureName);
			pOutputTargetTexture->SetUsage(METextureUsage::ERenderBack);
			pOutputTargetTexture->SetLayout(METextureLayout::ERGBA8);
			pOutputTargetTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pOutputTargetTexture->SetSize(Vector2(1.0f, 1.0f));
		}

		MRenderGraphNodeOutput* pOutputTarget = pGBufferNode->AppendOutput();
		pOutputTarget->SetClear(true);
		pOutputTarget->SetClearColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		pOutputTarget->SetRenderTexture(pOutputTargetTexture);
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

	MRenderGraphNodeOutput* pOutputDepth = pGBufferNode->AppendOutput();
	pOutputDepth->SetClear(true);
	pOutputDepth->SetRenderTexture(pOutputDepthTexture);


	pRenderGraph->SetFinalOutput(pGBufferNode->GetOutput(0));
	pGBufferNode->BindRenderFunction(std::bind(&MDeferredGBufferWork::Render, this, std::placeholders::_1));
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
