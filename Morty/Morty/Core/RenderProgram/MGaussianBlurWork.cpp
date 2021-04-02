#include "MGaussianBlurWork.h"
#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MForwardPostProcessProgram.h"

#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

M_OBJECT_IMPLEMENT(MGaussianBlurWork, MIPostProcessWork)


class MGaussianBlurGraphNode : public MRenderGraphNode
{
public:
	MGaussianBlurGraphNode();
	virtual ~MGaussianBlurGraphNode();

	virtual void UpdateBuffer(MIDevice* pDevice) override;

	virtual void DestroyBuffer(MIDevice* pDevice) override;

	MRenderPass* GetTempRenderPass() { return m_pTempRenderPass; }

protected:


	MRenderPass* m_pTempRenderPass;
};

MGaussianBlurGraphNode::MGaussianBlurGraphNode()
	: MRenderGraphNode()
	, m_pTempRenderPass(new MRenderPass())
{

}

MGaussianBlurGraphNode::~MGaussianBlurGraphNode()
{
	if (m_pTempRenderPass)
	{
		delete m_pTempRenderPass;
		m_pTempRenderPass = nullptr;
	}
}

void MGaussianBlurGraphNode::UpdateBuffer(MIDevice* pDevice)
{
	//chean Renderpass.
	m_pRenderPass->DestroyBuffer(pDevice);
	m_pRenderPass->m_vBackDesc = {};
	MFrameBuffer* pFrameBuffer = m_pRenderPass->GetFrameBuffer();
	pFrameBuffer->vBackTextures = {};
	pFrameBuffer->pDepthTexture = nullptr;

	m_pTempRenderPass->DestroyBuffer(pDevice);
	m_pTempRenderPass->m_vBackDesc = {};
	MFrameBuffer* pTempFrameBuffer = m_pTempRenderPass->GetFrameBuffer();
	pTempFrameBuffer->vBackTextures = {};
	pTempFrameBuffer->pDepthTexture = nullptr;

	if (m_vOutputTextures.size() < 2)
	{
		//TODO error.
		return;
	}

	if (MRenderGraphNodeOutput* pOutput = m_vOutputTextures[0])
	{
		if (MRenderGraphTexture* pRenderTexture = pOutput->GetRenderTexture())
		{
			if (pRenderTexture->GetUsage() == METextureUsage::ERenderBack)
			{
				MFrameBuffer* pFrameBuffer = m_pRenderPass->GetFrameBuffer();
				pFrameBuffer->vBackTextures.push_back(pRenderTexture->GetRenderTexture());

				m_pRenderPass->m_vBackDesc.push_back({});
				m_pRenderPass->m_vBackDesc.back().bClearWhenRender = pOutput->GetClear();
				m_pRenderPass->m_vBackDesc.back().cClearColor = pOutput->GetClearColor();
			}
		}
	}
	if (MRenderGraphNodeOutput* pOutput = m_vOutputTextures[1])
	{
		if (MRenderGraphTexture* pRenderTexture = pOutput->GetRenderTexture())
		{
			if (pRenderTexture->GetUsage() == METextureUsage::ERenderBack)
			{
				MFrameBuffer* pFrameBuffer = m_pTempRenderPass->GetFrameBuffer();
				pFrameBuffer->vBackTextures.push_back(pRenderTexture->GetRenderTexture());

				m_pTempRenderPass->m_vBackDesc.push_back({});
				m_pTempRenderPass->m_vBackDesc.back().bClearWhenRender = pOutput->GetClear();
				m_pTempRenderPass->m_vBackDesc.back().cClearColor = pOutput->GetClearColor();
			}
		}
	}

	m_pRenderPass->GenerateBuffer(pDevice);
	m_pTempRenderPass->GenerateBuffer(pDevice);
}

void MGaussianBlurGraphNode::DestroyBuffer(MIDevice* pDevice)
{
	m_pRenderPass->DestroyBuffer(pDevice);
	m_pTempRenderPass->DestroyBuffer(pDevice);
}

MGaussianBlurWork::MGaussianBlurWork()
    : MIPostProcessWork()
	, m_strGraphNodeName("")
	, m_pRenderProgram(nullptr)
	, m_pScreenDrawMesh(nullptr)
    , m_aMaterial()
	, m_fBlurRadius(1.0f)
	, m_unIteration(6)
{
}

MGaussianBlurWork::~MGaussianBlurWork()
{
}

void MGaussianBlurWork::Update(MRenderGraphNode* pGraphNode)
{
	
}

void MGaussianBlurWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardPostProcessProgram* pRenderProgram = dynamic_cast<MForwardPostProcessProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;

	MRenderInfo& info = pRenderProgram->GetRenderInfo();

	MRenderGraphNodeOutput* pOutput0 = pGraphNode->GetOutput(0);
	if (!pOutput0)
		return;

	MRenderGraphNodeOutput* pOutput1 = pGraphNode->GetOutput(0);
	if (!pOutput1)
		return;

	MRenderGraphTexture* pOutputTexture0 = pOutput0->GetRenderTexture();
	if (!pOutputTexture0)
		return;

	MRenderGraphTexture* pOutputTexture1 = pOutput1->GetRenderTexture();
	if (!pOutputTexture1)
		return;

	MRenderGraphNodeInput* pInput = pGraphNode->GetInput(0);
	if (!pInput)
		return;

	MRenderGraphTexture* pInputTexture = pInput->GetLinkedTexture();
	if (!pInputTexture)
		return;

	MGaussianBlurGraphNode* pTypedGraphNode = pGraphNode->DynamicCast<MGaussianBlurGraphNode>();
	if (!pTypedGraphNode)
		return;


	MRenderPass* pRenderPass = pGraphNode->GetRenderPass();


	UpdateShaderSharedParams(pGraphNode, info);
	for (uint32_t i = 0; i < 2 * m_unIteration; ++i)
	{

		MMaterial* pMaterial = nullptr;
		if (MShaderParamSet* pMaterialParamSet = m_aMaterial[i % 2]->GetMaterialParamSet())
		{
			if (i == 0)
			{
				MIRenderTexture* pBackTexture = pInputTexture->GetRenderTexture();
				info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, { pBackTexture });

				pMaterial = m_aMaterial[2];
			}
			else if (i % 2 == 0)
			{
				MIRenderTexture* pBackTexture = pOutputTexture1->GetRenderTexture();
				info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, { pBackTexture });

				pMaterial = m_aMaterial[0];
			}
			else
			{
				MIRenderTexture* pBackTexture = pOutputTexture0->GetRenderTexture();
				info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, { pBackTexture });

				pMaterial = m_aMaterial[1];
			}

		}

		if (i % 2 == 0)
			info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pTypedGraphNode->GetRenderPass(), info.unFrameIndex);
		else
			info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pTypedGraphNode->GetTempRenderPass(), info.unFrameIndex);

		Vector2 v2OutputSize = pOutputTexture0->GetOutputSize();
		info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));
		info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));

		if (info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
		{
			info.pRenderer->DrawMesh(info.pPrimaryCommand, m_pScreenDrawMesh);
		}

		info.pRenderer->EndRenderPass(info.pPrimaryCommand);
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
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(MGlobal::DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MGaussianBlurWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(MGlobal::DEFAULT_MESH_SCREEN_DRAW);
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



	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode<MGaussianBlurGraphNode>("Gaussian_Post"))
	{
		m_strGraphNodeName = pPostProcessNode->GetNodeName();

		pPostProcessNode->AppendInput();

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			MRenderGraphTexture* pGaussianOutputTexture = pRenderGraph->AddRenderGraphTexture("Gaussian_Post_0");
			pGaussianOutputTexture->SetLayout(pOutputTargetTexture->GetLayout());
			pGaussianOutputTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pGaussianOutputTexture->SetSize(Vector2(1.0f, 1.0f));
			pGaussianOutputTexture->SetUsage(pOutputTargetTexture->GetUsage());


			pOutput->SetRenderTexture(pGaussianOutputTexture);
			pOutput->SetClear(true);
			pOutput->SetClearColor(m_pRenderProgram->GetClearColor());
		}

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			MRenderGraphTexture* pGaussianOutputTexture = pRenderGraph->AddRenderGraphTexture("Gaussian_Post_1");
			pGaussianOutputTexture->SetLayout(pOutputTargetTexture->GetLayout());
			pGaussianOutputTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
			pGaussianOutputTexture->SetSize(Vector2(1.0f, 1.0f));
			pGaussianOutputTexture->SetUsage(pOutputTargetTexture->GetUsage());


			pOutput->SetRenderTexture(pGaussianOutputTexture);
			pOutput->SetClear(true);
			pOutput->SetClearColor(m_pRenderProgram->GetClearColor());
		}

		pPostProcessNode->BindRenderFunction(std::bind(&MGaussianBlurWork::Render, this, std::placeholders::_1));
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
