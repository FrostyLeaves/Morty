#include "MGaussianBlurWork.h"
#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MForwardPostProcessProgram.h"

#include "MRenderGraph.h"

M_OBJECT_IMPLEMENT(MGaussianBlurWork, MIPostProcessWork)

MGaussianBlurWork::MGaussianBlurWork()
    : MIPostProcessWork()
	, m_strGraphNodeName("")
    , m_aMaterial()
	, m_fBlurRadius(1.0f)
	, m_unIteration(6)
{
}

MGaussianBlurWork::~MGaussianBlurWork()
{
}

void MGaussianBlurWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardPostProcessProgram* pRenderProgram = dynamic_cast<MForwardPostProcessProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;

	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	MRenderGraphNodeInput* pInput = pGraphNode->GetInput(0);
	if (!pInput)
		return;

	MRenderGraphTexture* pInputTexture = pInput->GetLinkedTexture();
	if (!pInputTexture)
		return;

	MRenderGraphTexture* pOutput0Texture = pGraphNode->GetOutput(0)->GetRenderTexture();
	MRenderGraphTexture* pOutput1Texture = pGraphNode->GetOutput(1)->GetRenderTexture();


	UpdateShaderSharedParams(pGraphNode, info);

	for (uint32_t i = 0; i < 2 * m_unIteration; ++i)
	{
		MMaterial* pMaterial = nullptr;
		if (MShaderParamSet* pMaterialParamSet = m_aMaterial[i % 2]->GetMaterialParamSet())
		{
			if (i == 0)
			{
				MIRenderTexture* pBackTexture = pInputTexture->GetRenderTexture();
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[2];
			}
			else if (i % 2 == 0)
			{
				MIRenderTexture* pBackTexture = pOutput1Texture->GetRenderTexture();
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[0];
			}
			else
			{
				MIRenderTexture* pBackTexture = pOutput0Texture->GetRenderTexture();
				info.pRenderer->SetRenderToTextureBarrier({ pBackTexture });

				pMaterial = m_aMaterial[1];
			}

		}

		info.pRenderer->BeginRenderPass(pGraphNode->GetRenderPass(), info.unFrameIndex);

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

void MGaussianBlurWork::UpdateShaderSharedParams(MRenderGraphNode* pGraphNode, MRenderInfo& info)
{
	MRenderGraphTexture* pInputTexture = pGraphNode->GetInput(0)->GetLinkedTexture();
	MRenderGraphTexture* pOutput0Texture = pGraphNode->GetOutput(0)->GetRenderTexture();
	MRenderGraphTexture* pOutput1Texture = pGraphNode->GetOutput(1)->GetRenderTexture();


	Vector2 v2ViewportSize = info.pViewport->GetSize();
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[0]->GetMaterialParamSet())
	{
		MIRenderTexture* pBackTexture = pOutput1Texture->GetRenderTexture();
		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.x = m_fBlurRadius / v2ViewportSize.x;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[1]->GetMaterialParamSet())
	{
		MIRenderTexture* pBackTexture = pOutput0Texture->GetRenderTexture();
		pMaterialParamSet->m_vTextures[0]->pTexture = pBackTexture;
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		Vector2 v2BlurOffset;
		v2BlurOffset.y = m_fBlurRadius / v2ViewportSize.y;
		pMaterialParamSet->m_vParams[0]->var = v2BlurOffset;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}
	if (MShaderParamSet* pMaterialParamSet = m_aMaterial[2]->GetMaterialParamSet())
	{
		MIRenderTexture* pBackTexture = pInputTexture->GetRenderTexture();
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

	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MRenderGraphTexture* aTempOutputTextures[2] = {
		pRenderGraph->AddRenderGraphTexture("Gaussian_Post_A"),
		pRenderGraph->AddRenderGraphTexture("Gaussian_Post_B"),
	};

	for(size_t i = 0; i < 2; ++i)
	{
		aTempOutputTextures[i]->SetLayout(pOutputTargetTexture->GetLayout());
		aTempOutputTextures[i]->SetSize(pOutputTargetTexture->GetSize());
		aTempOutputTextures[i]->SetUsage(pOutputTargetTexture->GetUsage());
	}



	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode("Gaussian_Post"))
	{
		m_strGraphNodeName = pPostProcessNode->GetNodeName();

		pPostProcessNode->AppendInput();

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(aTempOutputTextures[0]);
			pOutput->SetClear(true);
			pOutput->SetClearColor(m_pRenderProgram->GetClearColor());
		}

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(aTempOutputTextures[1]);
			pOutput->SetClear(true);
			pOutput->SetClearColor(m_pRenderProgram->GetClearColor());
		}
	}

}

void MGaussianBlurWork::ReleaseGraph()
{

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
