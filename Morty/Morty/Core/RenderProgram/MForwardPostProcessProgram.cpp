#include "MForwardPostProcessProgram.h"

#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MForwardHDRWork.h"
#include "MGaussianBlurWork.h"
#include "MForwardRenderWork.h"
#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardPostProcessProgram, MForwardRenderProgram)

MForwardPostProcessProgram::MForwardPostProcessProgram()
    : MForwardRenderProgram()
	, m_bHDR_Enable(false)
	, m_pHDRPostProcessWork(nullptr)
	, m_vPostProcessWork()
	, m_aBackTexture()
	, m_aDepthTexture()
	, m_pTempRenderTarget(nullptr)
	, m_pScreenDrawRenderPass(nullptr)
	, m_pScreenDrawMesh(nullptr)
	, m_pScreenDrawMaterial(nullptr)
{
}

MForwardPostProcessProgram::~MForwardPostProcessProgram()
{
}

void MForwardPostProcessProgram::Render(MIRenderer* pRenderer, MViewport* pViewport)
{
	if (!pViewport)
		return;

	MRenderInfo info;
	memset(&info, 0, sizeof(MRenderInfo));

	info.fDelta = m_pEngine->GetInstantDelta();
	info.unFrameIndex = pRenderer->GetFrameIndex();
	info.pRenderTarget = m_pTempRenderTarget;
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCamera = pViewport->GetCamera();
	info.pScene = pViewport->GetScene();


	CheckRenderTargetSize(pViewport->GetSize());

	MForwardRenderProgram::Render(info);

	RenderPostProcess(info);

	RenderScreenMesh(info);

	pViewport->UnlockMatrix();
}

void MForwardPostProcessProgram::SetHighDynamicRangeEnable(const bool& bEnable)
{
	if (m_bHDR_Enable == bEnable)
		return;
	m_bHDR_Enable = bEnable;

	GetEngine()->GetDevice()->DestroyRenderPass(m_pScreenDrawRenderPass);

	METextureLayout eNewLayout = bEnable ? METextureLayout::ERGBA16 : METextureLayout::ERGBA8;

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (m_aBackTexture[i])
		{
			m_aBackTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			m_aBackTexture[i]->SetType(eNewLayout);
			m_aBackTexture[i]->GenerateBuffer(GetEngine()->GetDevice());
		}
	}

	if (bEnable && !m_pHDRPostProcessWork)
	{
		m_pHDRPostProcessWork = GetEngine()->GetObjectManager()->CreateObject<MForwardHDRWork>();
		m_pHDRPostProcessWork->Initialize(this);
	}
	else if (!bEnable && m_pHDRPostProcessWork)
	{
		m_pHDRPostProcessWork->DeleteLater();
		m_pHDRPostProcessWork = nullptr;
	}
}

void MForwardPostProcessProgram::RenderPostProcess(const MRenderInfo& info)
{
	Vector2 v2ViewportSize = info.pViewport->GetSize();

	uint32_t unFrameIdx = info.pRenderer->GetFrameIndex();

	MPostProcessRenderInfo cPostInfo;

	cPostInfo.fDelta = info.fDelta;
	cPostInfo.unFrameIndex = unFrameIdx;
	cPostInfo.pViewport = info.pViewport;
	cPostInfo.pRenderer = info.pRenderer;
	cPostInfo.pPrevLevelOutput = m_pTempRenderTarget->GetBackTexture(unFrameIdx)->at(0);

	if (m_pHDRPostProcessWork)
	{
		m_pHDRPostProcessWork->CheckRenderTargetSize(v2ViewportSize);
		m_pHDRPostProcessWork->Render(cPostInfo);

		cPostInfo.pPrevLevelOutput = m_pHDRPostProcessWork->GetRenderTarget()->GetBackTexture(unFrameIdx)->at(0);
	}

	for (MIPostProcessWork* pPostProcess : m_vPostProcessWork)
	{
		pPostProcess->CheckRenderTargetSize(v2ViewportSize);
		pPostProcess->Render(cPostInfo);

		cPostInfo.pPrevLevelOutput = pPostProcess->GetRenderTarget()->GetBackTexture(unFrameIdx)->at(0);
	}
}

void MForwardPostProcessProgram::RenderScreenMesh(const MRenderInfo& info)
{
	MTextureRenderTarget* pTextureRT = nullptr;

	if (!pTextureRT && !m_vPostProcessWork.empty())
		m_vPostProcessWork.back()->GetRenderTarget();

	if (!pTextureRT && m_pHDRPostProcessWork)
		pTextureRT = m_pHDRPostProcessWork->GetRenderTarget();

	if (!pTextureRT)
		return;

	uint32_t unFrameIndex = info.pRenderer->GetFrameIndex();

	info.pRenderer->SetRenderToTextureBarrier({ pTextureRT->GetBackTexture(unFrameIndex)->at(0) });

	info.pRenderer->BeginRenderPass(m_pScreenDrawRenderPass, GetRenderTarget());

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pScreenDrawMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = pTextureRT->GetBackTexture(unFrameIndex)->at(0);
		pMaterialParamSet->m_vTextures[0]->SetDirty();
	}

	if (info.pRenderer->SetUseMaterial(m_pScreenDrawMaterial))
	{
		info.pRenderer->DrawMesh(m_pScreenDrawMesh);
	}

	info.pRenderer->EndRenderPass();
}

void MForwardPostProcessProgram::CheckRenderTargetSize(const Vector2& v2ViewportSize)
{
	if (m_pTempRenderTarget)
	{
		Vector2 v2Size = m_pTempRenderTarget->GetSize();
		if (v2Size.x != v2ViewportSize.x || v2Size.y != v2ViewportSize.y)
		{
			for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
			{
				m_aBackTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aBackTexture[i]->SetSize(v2ViewportSize);
				m_aBackTexture[i]->GenerateBuffer(GetEngine()->GetDevice());

				m_aDepthTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
				m_aDepthTexture[i]->SetSize(v2ViewportSize);
				m_aDepthTexture[i]->GenerateBuffer(GetEngine()->GetDevice());
			}

			m_pTempRenderTarget->Resize(v2ViewportSize);
		}
	}
}

void MForwardPostProcessProgram::Initialize()
{
    Super::Initialize();

	InitializeMesh();
	InitializeMaterial();
	InitializeRenderTarget();
	InitializeRenderPass();

	//AppendPostProcess<MGaussianBlurWork>();
}

void MForwardPostProcessProgram::Release()
{
	if (m_pHDRPostProcessWork)
	{
		m_pHDRPostProcessWork->DeleteLater();
		m_pHDRPostProcessWork = nullptr;
	}

	for (MIPostProcessWork* pPostProcess : m_vPostProcessWork)
	{
		pPostProcess->DeleteLater();
		pPostProcess = nullptr;
	}

	m_vPostProcessWork.clear();

	ReleaseRenderPass();
	ReleaseRenderTarget();
	ReleaseMaterial();
	ReleaseMesh();

    Super::Release();
}

void MForwardPostProcessProgram::InitializeMesh()
{
	if (MMeshResource* pMeshResource = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW))
	{
		m_pScreenDrawMesh = pMeshResource->GetMesh();
		pMeshResource->AddRef();
	}
}

void MForwardPostProcessProgram::ReleaseMesh()
{
	if (m_pScreenDrawMesh)
	{
		if (MMeshResource* pMeshResource = GetEngine()->GetResourceManager()->LoadVirtualResource<MMeshResource>(DEFAULT_MESH_SCREEN_DRAW))
		{
			pMeshResource->SubRef();
		}

		m_pScreenDrawMesh = nullptr;
	}
}

void MForwardPostProcessProgram::InitializeMaterial()
{
	m_pScreenDrawMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterial>();
	m_pScreenDrawMaterial->LoadVertexShader("./Shader/post_process_basic.mvs");
	m_pScreenDrawMaterial->LoadPixelShader("./Shader/post_process_basic.mps");
	m_pScreenDrawMaterial->AddRef();
}

void MForwardPostProcessProgram::ReleaseMaterial()
{
	if (m_pScreenDrawMaterial)
	{
		m_pScreenDrawMaterial->SubRef();
		m_pScreenDrawMaterial = nullptr;
	}
}

void MForwardPostProcessProgram::InitializeRenderPass()
{
	m_pScreenDrawRenderPass = new MRenderPass();

	if (!GetRenderTarget())
	{
		MLogManager::GetInstance()->Error("MForwardPostProcessProgram::InitializeRenderPass error: rt == nullptr");
		return;
	}

	//Init RenderPass
	m_pScreenDrawRenderPass->m_vBackDesc.push_back(MRenderPass::MTargetDesc());
	m_pScreenDrawRenderPass->m_vBackDesc.back().bClearWhenRender = true;
	m_pScreenDrawRenderPass->m_vBackDesc.back().cClearColor = GetClearColor();

	m_pScreenDrawRenderPass->m_DepthDesc.bClearWhenRender = true;
}

void MForwardPostProcessProgram::ReleaseRenderPass()
{
	if (m_pScreenDrawRenderPass)
	{
		GetEngine()->GetDevice()->DestroyRenderPass(m_pScreenDrawRenderPass);
		delete m_pScreenDrawRenderPass;
		m_pScreenDrawRenderPass = nullptr;
	}
}

void MForwardPostProcessProgram::InitializeRenderTarget()
{
	m_pTempRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();

	METextureLayout eNewLayout = m_bHDR_Enable ? METextureLayout::ERGBA16 : METextureLayout::ERGBA8;

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MRenderBackTexture* pBackTexture = new MRenderBackTexture();
		MRenderDepthTexture* pDepthTexture = new MRenderDepthTexture();

		pBackTexture->SetType(eNewLayout);

		m_aBackTexture[i] = pBackTexture;
		m_aDepthTexture[i] = pDepthTexture;
	}

	m_pTempRenderTarget->SetBackTexture(m_aBackTexture, 0);
	m_pTempRenderTarget->SetDepthTexture(m_aDepthTexture);

	m_pTempRenderTarget->Resize(Vector2(MSHADOW_TEXTURE_SIZE, MSHADOW_TEXTURE_SIZE));
}

void MForwardPostProcessProgram::ReleaseRenderTarget()
{
	if (m_pTempRenderTarget)
	{
		m_pTempRenderTarget->DeleteLater();
		m_pTempRenderTarget = nullptr;
	}

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (m_aBackTexture[i])
		{
			m_aBackTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			delete m_aBackTexture[i];
			m_aBackTexture[i] = nullptr;
		}

		if (m_aDepthTexture[i])
		{
			m_aDepthTexture[i]->DestroyBuffer(GetEngine()->GetDevice());
			delete m_aDepthTexture[i];
			m_aDepthTexture[i] = nullptr;
		}
	}
}

