#include "MGaussianBlurWork.h"
#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MRenderGraph.h"

M_OBJECT_IMPLEMENT(MGaussianBlurWork, MStandardPostProcessWork)

MGaussianBlurWork::MGaussianBlurWork()
    : MStandardPostProcessWork()
    , m_aMaterial()
	, m_fBlurRadius(1.0f)
	, m_unIteration(6)
{
}

MGaussianBlurWork::~MGaussianBlurWork()
{
}
void MGaussianBlurWork::Render(MRenderGraphNode* pGraphNode, MRenderInfo& info)
{
	UpdateShaderSharedParams(info);

	for (uint32_t i = 0; i < 2 * m_unIteration; ++i)
	{
		MMaterial* pMaterial = nullptr;
		if (MShaderParamSet* pMaterialParamSet = m_aMaterial[i % 2]->GetMaterialParamSet())
		{
			if (i == 0)
			{
				MIRenderTexture* pBackTexture = info.pPrevLevelOutput;
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[2];
			}
			else if (i % 2 == 0)
			{
//				MIRenderTexture* pBackTexture = m_aTempRenderTarget[1]->GetBackTexture(info.unFrameIndex)->at(0);
//				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[0];
			}
			else
			{
//				MIRenderTexture* pBackTexture = m_aTempRenderTarget[0]->GetBackTexture(info.unFrameIndex)->at(0);
//				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[1];
			}

		}

		info.pRenderer->BeginRenderPass(m_pTempRenderPass, m_aTempRenderTarget[i % 2]);

		Vector2 v2LeftTop = info.pViewport->GetLeftTop();
		Vector2 v2ViewportSize = info.pViewport->GetSize();
		info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, v2ViewportSize.x, v2ViewportSize.y, 0.0f, 1.0f);

		if (info.pRenderer->SetUseMaterial(pMaterial))
		{
			info.pRenderer->DrawMesh(m_pScreenDrawMesh);
		}

		info.pRenderer->EndRenderPass();
	}
}

void MGaussianBlurWork::UpdateShaderSharedParams(MPostProcessRenderInfo& info)
{
	Vector2 v2ViewportSize = info.pViewport->GetSize();
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[0]->GetMaterialParamSet())
	{
//		MIRenderTexture* pBackTexture = m_aTempRenderTarget[1]->GetBackTexture(info.unFrameIndex)->at(0);
//		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.x = m_fBlurRadius / v2ViewportSize.x;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[1]->GetMaterialParamSet())
	{
//		MIRenderTexture* pBackTexture = m_aTempRenderTarget[0]->GetBackTexture(info.unFrameIndex)->at(0);
//		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.y = m_fBlurRadius / v2ViewportSize.y;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[2]->GetMaterialParamSet())
	{
		MIRenderTexture* pBackTexture = info.pPrevLevelOutput;
		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.x = m_fBlurRadius / v2ViewportSize.x;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}

}

void MGaussianBlurWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MGaussianBlurWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MGaussianBlurWork::InitializeGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MStandardPostProcessWork::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	MRenderGraphNode* pFinalNode = pRenderGraph->GetFinalNode();
	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MString strNameHead("Standard_Post_");
	MString strTempOutputTextureName;
	MString strTempOutputNodeName;

	{
		int i = 0;
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

void MGaussianBlurWork::ReleaseGraph()
{

}

void MGaussianBlurWork::InitializeRenderTargets()
{
	for (uint32_t nRtIdx = 0; nRtIdx < 2; ++nRtIdx)
	{
		m_aTempRenderTarget[nRtIdx] = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

		for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
		{
			MRenderTexture* pBackTexture = new MRenderTexture();
			m_aBackTexture[nRtIdx][i] = pBackTexture;
		}

//		m_aTempRenderTarget[nRtIdx]->SetBackTexture(m_aBackTexture[nRtIdx], 0);
	}
}

void MGaussianBlurWork::InitializeRenderPass()
{

	//Init RenderPass
	m_pTempRenderPass = new MRenderPass();
	m_pTempRenderPass->m_vBackDesc.push_back(MPassTargetDescription());
	m_pTempRenderPass->m_vBackDesc.back().bClearWhenRender = true;
	m_pTempRenderPass->m_vBackDesc.back().cClearColor = m_pRenderProgram->GetClearColor();

	m_pTempRenderPass->m_DepthDesc.bClearWhenRender = true;
}

void MGaussianBlurWork::Initialize(MIRenderProgram* pRenderProgram)
{
	Super::Initialize(pRenderProgram);

	m_pRenderProgram = pRenderProgram;

	InitializeMesh();
	InitializeMaterial();
	InitializeGraph();
}

void MGaussianBlurWork::Release()
{
	ReleaseGraph();
	ReleaseMaterial();
	ReleaseMesh();

	Super::Release();
}

void MGaussianBlurWork::InitializeMaterial()
{
	MResource* pVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/gaussian_blur.mvs");
	MResource* pPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/gaussian_blur.mps");

	for (uint32_t i = 0; i < 3; ++i)
	{
		m_aMaterial[i] = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();

		m_aMaterial[i]->LoadVertexShader(pVSResource);
		m_aMaterial[i]->LoadPixelShader(pPSResource);

		m_aMaterial[i]->AddRef();
	}
}

void MGaussianBlurWork::ReleaseMaterial()
{
	for (uint32_t i = 0; i < 3; ++i)
	{
		if (m_aMaterial[i])
		{
			m_aMaterial[i]->SubRef();
			m_aMaterial[i] = nullptr;
		}
	}
}
