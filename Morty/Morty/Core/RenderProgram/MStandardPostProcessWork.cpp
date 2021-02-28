#include "MStandardPostProcessWork.h"
#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MRenderGraph.h"
#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MStandardPostProcessWork, MIPostProcessWork)

MStandardPostProcessWork::MStandardPostProcessWork()
	: MIPostProcessWork()
	, m_strGraphNodeName("")
	, m_pRenderProgram(nullptr)
	, m_pScreenDrawMesh(nullptr)
{
}

MStandardPostProcessWork::~MStandardPostProcessWork()
{
}

void MStandardPostProcessWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeMesh();
	InitializeRenderGraph();
}

void MStandardPostProcessWork::Release()
{
	ReleaseRenderGraph();
	ReleaseMesh();
}

void MStandardPostProcessWork::Render(MRenderGraphNode* pGraphNode)
{

}

void MStandardPostProcessWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MStandardPostProcessWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MStandardPostProcessWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MStandardPostProcessWork::InitializeRenderGraph error: rg == nullptr");
		return;
	}


	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MRenderGraphTexture* pTempOutputTexture = pRenderGraph->AddRenderGraphTexture("Standard_Post");
	if (pTempOutputTexture)
	{
		pTempOutputTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pTempOutputTexture->SetSize(Vector2(1.0f, 1.0f));
		pTempOutputTexture->SetUsage(pOutputTargetTexture->GetUsage());
	}
	
	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode("Standard_Post"))
	{
		m_strGraphNodeName = pPostProcessNode->GetNodeName();

		pPostProcessNode->AppendInput();

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(pTempOutputTexture);
		}

		pPostProcessNode->BindRenderFunction(std::bind(&MStandardPostProcessWork::Render, this, std::placeholders::_1));
	}

}

void MStandardPostProcessWork::ReleaseRenderGraph()
{

}
