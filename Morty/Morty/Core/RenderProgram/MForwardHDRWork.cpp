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


#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

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
	, m_pInput(nullptr)
	, m_pOutput(nullptr)
	, m_aLumTexture()
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
	MRenderInfo& info = *m_pRenderProgram->GetRenderInfo();

	MRenderGraphNodeOutput* pOutput = pGraphNode->GetOutput(0);
	if (!pOutput)
		return;

	MRenderGraphTexture* pOutputTexture = pOutput->GetRenderTexture();
	if (!pOutputTexture)
		return;

	MRenderGraphNodeInput* pInput =  pGraphNode->GetInput(0);
	if (!pInput)
		return;

	MRenderGraphTexture* pInputTexture = pInput->GetLinkedTexture();
	if (!pInputTexture)
		return;

	info.pRenderer->CopyImageBuffer(info.pPrimaryCommand, pInputTexture->GetRenderTexture(), m_aLumTexture[info.unFrameIndex]);
	info.pRenderer->UpdateMipmaps(info.pPrimaryCommand, m_aLumTexture[info.unFrameIndex]->GetBuffer(info.unFrameIndex));

 	uint32_t unMipIdx = m_aLumTexture[info.unFrameIndex]->GetBuffer(info.unFrameIndex)->m_unMipmaps;
 	unMipIdx = unMipIdx >= 3 ? unMipIdx - 3 : unMipIdx;

	info.pRenderer->DownloadTexture(info.pPrimaryCommand, m_aLumTexture[info.unFrameIndex], unMipIdx, [this](void* pImageData, const Vector2& size) {

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

	info.pRenderer->SetRenderToTextureBarrier(info.pPrimaryCommand, { pInputTexture->GetRenderTexture() });

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);


	Vector2 v2OutputSize = pOutputTexture->GetOutputSize();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, v2OutputSize.x, v2OutputSize.y));

	if (MShaderParamSet* pMaterialParamSet = m_pHDRMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = pInputTexture->GetRenderTexture();
		pMaterialParamSet->m_vTextures[0]->SetDirty();

		m_fAdaptLum += (m_fAverageLum * m_fAdjustLum - m_fAdaptLum) * (1.0f - pow(0.98f, 30 * info.fDelta));
		pMaterialParamSet->m_vParams[0]->var = m_fAdaptLum;
		pMaterialParamSet->m_vParams[0]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_pHDRMaterial))
	{
		info.pRenderer->DrawMesh(info.pPrimaryCommand, m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MForwardHDRWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	Super::Initialize(pRenderProgram);

	InitializeCopyTarget();
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
	ReleaseCopyTarget();

	Super::Release();
}

MRenderGraphNodeInput* MForwardHDRWork::GetInput()
{
	return m_pInput;
}

MRenderGraphNodeOutput* MForwardHDRWork::GetOutput()
{
	return m_pOutput;
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
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(MGlobal::DEFAULT_MESH_SCREEN_DRAW);
	pScreenMeshRes->AddRef();

	m_pScreenDrawMesh = pScreenMeshRes->GetMesh();
}

void MForwardHDRWork::ReleaseMesh()
{
	MMeshResource* pScreenMeshRes = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(MGlobal::DEFAULT_MESH_SCREEN_DRAW);
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

	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->GetFinalOutputTexture();


	MRenderGraphTexture* pTempOutputHDRTexture = pRenderGraph->AddRenderGraphTexture("HDR_Post");
	if (pTempOutputHDRTexture)
	{
		pTempOutputHDRTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputHDRTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pTempOutputHDRTexture->SetSize(Vector2(1.0f, 1.0f));
		pTempOutputHDRTexture->SetUsage(pOutputTargetTexture->GetUsage());
	}

	MRenderGraphTexture* pTempOutputHighLightTexture = pRenderGraph->AddRenderGraphTexture("HDR_Post_HL");
	if (pTempOutputHighLightTexture)
	{
		pTempOutputHighLightTexture->SetLayout(pOutputTargetTexture->GetLayout());
		pTempOutputHighLightTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pTempOutputHighLightTexture->SetSize(Vector2(1.0f, 1.0f));
		pTempOutputHighLightTexture->SetUsage(pOutputTargetTexture->GetUsage());
	}

	if (MRenderGraphNode* pPostProcessNode = pRenderGraph->AddRenderGraphNode("HDR_Post"))
	{
		m_pInput = pPostProcessNode->AppendInput();

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
		
		m_pOutput = pCombineNode->GetOutput(0);
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
		m_aLumTexture[i]->SetSize(Vector2(512, 512));
		m_aLumTexture[i]->GenerateBuffer(GetEngine()->GetDevice());
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
