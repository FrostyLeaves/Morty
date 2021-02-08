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


	MRenderGraphNode* pFinalNode = pRenderGraph->GetFinalNode();
	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MString strTempOutputTextureName;
	MString strTempOutputNodeName;

	{
		int i = 0;
		MString strNameHead("Standard_Post_");
		strTempOutputTextureName = strNameHead + MStringHelper::ToString(i);
		while (pRenderGraph->FindRenderGraphTexture(strTempOutputTextureName))
		{
			strTempOutputTextureName = strNameHead + MStringHelper::ToString(++i);
		}
	}

	MRenderGraphTexture* pTempOutputTexture = pRenderGraph->AddRenderGraphTexture(strTempOutputTextureName);
	if (pTempOutputTexture)
	{
		pTempOutputTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputTexture->SetSize(pTempOutputTexture->GetSize());
		pTempOutputTexture->SetUsage(pTempOutputTexture->GetUsage());
		pRenderGraph->SetFinalOutputTexture(pTempOutputTexture);
	}

	{
		int i = 0;
		MString strNameHead("Standard_Post_");
		strTempOutputNodeName = strNameHead + MStringHelper::ToString(i);
		while (pRenderGraph->FindRenderGraphNode(strTempOutputNodeName))
		{
			strTempOutputNodeName = strNameHead + MStringHelper::ToString(++i);
		}
	}
	
	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode(strTempOutputNodeName))
	{
		if (MRenderGraphNodeInput* pInput = pPostProcessNode->AppendInput())
		{
			pInput->LinkTo(pFinalNode->GetOutput(0));
		}

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(pTempOutputTexture);
		}
	}

}

void MStandardPostProcessWork::ReleaseRenderGraph()
{

}
