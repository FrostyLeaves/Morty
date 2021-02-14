#include "MForwardHDRWork.h"

#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

#include "MCombineWork.h"
#include "MGaussianBlurWork.h"
#include "MForwardPostProcessProgram.h"

M_OBJECT_IMPLEMENT(MForwardHDRWork, MIPostProcessWork)

float ConvertHalfToFloat(const char16_t& h)
{
	int fs, fe, fm, rlt;

	fs = (h & 0x8000) << 16;

	fe = (h & 0x7c00) >> 10;
	fe = fe + 0x70;
	fe = fe << 23;

	fm = (h & 0x03ff) << 13;

	rlt = fs | fe | fm;
	return *((float*)&rlt);
}

MForwardHDRWork::MForwardHDRWork()
    : MIPostProcessWork()
	, m_pRenderProgram(nullptr)
	, m_pScreenDrawMesh(nullptr)
	, m_pHDRMaterial(nullptr)
	, m_pGaussianBlurWork(nullptr)
	, m_pCombineWork(nullptr)
	, m_fAverageLum(1.0f)
	, m_fAdaptLum(0.0f)
	, m_fAdjustLum(1.0f)
{
}

MForwardHDRWork::~MForwardHDRWork()
{
}

void MForwardHDRWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardPostProcessProgram* pRenderProgram = dynamic_cast<MForwardPostProcessProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;

	MRenderInfo& info = pRenderProgram->GetRenderInfo();


	MRenderGraphNodeInput* pInput =  pGraphNode->GetInput(0);
	if (!pInput)
		return;

	MRenderGraphTexture* pInputTexture = pInput->GetLinkedTexture();
	if (!pInputTexture)
		return;

	info.pRenderer->CopyImageBuffer(pInputTexture->GetRenderTexture(), m_aLumTexture[info.unFrameIndex]);
	info.pRenderer->UpdateMipmaps(m_aLumTexture[info.unFrameIndex]->GetBuffer(info.unFrameIndex));

	uint32_t unMipIdx = m_aLumTexture[info.unFrameIndex]->GetBuffer(info.unFrameIndex)->m_unMipmaps;
	unMipIdx = unMipIdx >= 3 ? unMipIdx - 3 : unMipIdx;

	info.pRenderer->DownloadTexture(m_aLumTexture[info.unFrameIndex], unMipIdx, [this](void* pImageData, const Vector2& size) {

		size_t unPixelSize = static_cast<size_t>(size.x) * static_cast<size_t>(size.y);
		float flum = 0.0f;
		m_fAverageLum = 0.0f;
		char16_t* iter = (char16_t*)pImageData;
		char16_t* end = iter + (unPixelSize * 4);

		for (iter; iter < end; iter += 4)
		{
			const float r = ConvertHalfToFloat(*(iter));
			const float g = ConvertHalfToFloat(*(iter + 1));
			const float b = ConvertHalfToFloat(*(iter + 2));
			flum = r * 0.27f + g * 0.67f + b * 0.06f + 1e-6f;
			m_fAverageLum += log(flum);
		}

		m_fAverageLum /= unPixelSize;
		m_fAverageLum = exp(m_fAverageLum);
		});

	info.pRenderer->SetRenderToTextureBarrier({ pInputTexture->GetRenderTexture() });

	info.pRenderer->BeginRenderPass(pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pHDRMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = pInputTexture->GetRenderTexture();
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		m_fAdaptLum += (m_fAverageLum * m_fAdjustLum - m_fAdaptLum) * (1.0f - pow(0.98f, 30 * info.fDelta));
		pMaterialParamSet->m_vParams[0]->var = m_fAdaptLum;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(m_pHDRMaterial))
	{
		info.pRenderer->DrawMesh(m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass();
}

void MForwardHDRWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	Super::Initialize(pRenderProgram);

	InitializeMesh();
	InitializeMaterial();


	if (!m_pGaussianBlurWork)
	{
		m_pGaussianBlurWork = GetEngine()->GetObjectManager()->CreateObject<MGaussianBlurWork>();
		m_pGaussianBlurWork->Initialize(pRenderProgram);
	}

	if (!m_pCombineWork)
	{
		m_pCombineWork = GetEngine()->GetObjectManager()->CreateObject<MCombineWork>();
		m_pCombineWork->Initialize(pRenderProgram);
	}

	InitializeRenderGraph();

}

void MForwardHDRWork::Release()
{
	if (m_pGaussianBlurWork)
	{
		m_pGaussianBlurWork->DeleteLater();
		m_pGaussianBlurWork = nullptr;
	}

	if (m_pCombineWork)
	{
		m_pCombineWork->DeleteLater();
		m_pCombineWork = nullptr;
	}

	ReleaseMaterial();
	ReleaseMesh();

	Super::Release();
}

void MForwardHDRWork::InitializeMaterial()
{
	m_pHDRMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();

	MResource* pVSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/post_process_basic.mvs");
	MResource* pPSResource = GetEngine()->GetResourceManager()->LoadResource("./Shader/high_dynamic_range.mps");

	m_pHDRMaterial->LoadVertexShader(pVSResource);
	m_pHDRMaterial->LoadPixelShader(pPSResource);

	m_pHDRMaterial->AddRef();
}

void MForwardHDRWork::ReleaseMaterial()
{
	m_pHDRMaterial->SubRef();
	m_pHDRMaterial = nullptr;
}

void MForwardHDRWork::InitializeMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MForwardHDRWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->SubRef();
}

void MForwardHDRWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardHDRWork::InitializeRenderGraph error: rg == nullptr");
		return;
	}


	MRenderGraphNode* pFinalNode = pRenderGraph->GetFinalNode();
	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MRenderGraphTexture* pTempOutputHDRTexture = pRenderGraph->AddRenderGraphTexture("HDR_Post");
	if (pTempOutputHDRTexture)
	{
		pTempOutputHDRTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputHDRTexture->SetSize(pOutputTargetTexture->GetSize());
		pTempOutputHDRTexture->SetUsage(pOutputTargetTexture->GetUsage());
	}

	MRenderGraphTexture* pTempOutputHighLightTexture = pRenderGraph->AddRenderGraphTexture("HDR_Post_HL");
	if (pTempOutputHighLightTexture)
	{
		pTempOutputHighLightTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputHighLightTexture->SetSize(pOutputTargetTexture->GetSize());
		pTempOutputHighLightTexture->SetUsage(pOutputTargetTexture->GetUsage());
	}

	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode("HDR_Post"))
	{
		if (MRenderGraphNodeInput* pInput = pPostProcessNode->AppendInput())
		{
			pInput->LinkTo(pFinalNode->GetOutput(0));
		}

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(pTempOutputHDRTexture);
			pOutput->SetClear(true);
			pOutput->SetClearColor(m_pRenderProgram->GetClearColor());
		}

		if (MRenderGraphNodeOutput* pOutput = pPostProcessNode->AppendOutput())
		{
			pOutput->SetRenderTexture(pTempOutputHighLightTexture);
			pOutput->SetClear(true);
			pOutput->SetClearColor(MColor::Black_T);

			
		}

		pPostProcessNode->BindRenderFunction(std::bind(&MForwardHDRWork::Render, this, std::placeholders::_1));

		MRenderGraphNode* pGaussNode = pRenderGraph->FindRenderGraphNode(m_pGaussianBlurWork->GetGraphNodeName());
		MRenderGraphNode* pCombineNode = pRenderGraph->FindRenderGraphNode(m_pCombineWork->GetGraphNodeName());

		pPostProcessNode->GetOutput(1)->LinkTo(pGaussNode->GetInput(0));
		pCombineNode->GetInput(0)->LinkTo(pPostProcessNode->GetOutput(0));
		pCombineNode->GetInput(1)->LinkTo(pGaussNode->GetOutput(0));
		
	}


}

void MForwardHDRWork::ReleaseRenderGraph()
{

}

void MForwardHDRWork::InitializeCopyTarget()
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_aLumTexture[i] = new MTexture();
		m_aLumTexture[i]->SetReadable(true);
		m_aLumTexture[i]->SetMipmapsEnable(true);
		m_aLumTexture[i]->SetType(METextureLayout::ERGBA16);
	}
}

void MForwardHDRWork::ReleaseCopyTarget()
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_aLumTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
		m_aLumTexture[i] = nullptr;
	}
}
