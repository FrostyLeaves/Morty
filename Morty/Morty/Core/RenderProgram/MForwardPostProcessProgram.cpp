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

void MForwardPostProcessProgram::Render(MIRenderer* pRenderer, const std::vector<MViewport*>& vViewports)
{
	if (vViewports.empty())
		return;

	//Only one viewport.
	MViewport* pViewport = vViewports[0];

	MRenderInfo info;
	memset(&info, 0, sizeof(MRenderInfo));

	CheckRenderTargetSize(pViewport->GetSize());

	info.unFrameIndex = pRenderer->GetFrameIndex();
	info.pRenderTarget = m_pTempRenderTarget;
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCamera = pViewport->GetCamera();
	info.pScene = pViewport->GetScene();

	pViewport->LockMatrix();

	GenerateRenderGroup(info);

	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->Render(info);
		info.pShadowMapTexture = m_pShadowMapWork->GetShadowMap(info.unFrameIndex);
	}

	if (m_pRenderWork)
	{
		m_pRenderWork->Render(info);
	}

	if (m_pTransparentWork)
	{
		m_pTransparentWork->Render(info);
	}

	RenderPostProcess(pRenderer, pViewport);

	RenderScreenMesh(pRenderer, pViewport);

	pViewport->UnlockMatrix();
}

void MForwardPostProcessProgram::RenderPostProcess(MIRenderer* pRenderer, MViewport* pViewport)
{
	MPostProcessRenderInfo cPostInfo;
	cPostInfo.unFrameIndex = pRenderer->GetFrameIndex();
	cPostInfo.pViewport = pViewport;
	cPostInfo.pRenderer = pRenderer;
	cPostInfo.pPrevLevel = m_pTempRenderTarget;

	for (MIPostProcessWork* pPostProcess : m_vPostProcessWork)
	{
		pPostProcess->CheckRenderTargetSize(pViewport->GetSize());
		pPostProcess->Render(cPostInfo);

		cPostInfo.pPrevLevel = pPostProcess->GetRenderTarget();
	}
}

void MForwardPostProcessProgram::RenderScreenMesh(MIRenderer* pRenderer, MViewport* pViewport)
{
	if (m_vPostProcessWork.empty())
		return;

	MTextureRenderTarget* pTextureRT = m_vPostProcessWork.back()->GetRenderTarget();
	if (!pTextureRT)
		return;

	uint32_t unFrameIndex = pRenderer->GetFrameIndex();

	pRenderer->BeginRenderPass(m_pScreenDrawRenderPass, GetRenderTarget());

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);

	if (MShaderParamSet* pMaterialParamSet = m_pScreenDrawMaterial->GetMaterialParamSet())
	{
		pMaterialParamSet->m_vTextures[0]->pTexture = pTextureRT->GetBackTexture(unFrameIndex)->at(0);
		pMaterialParamSet->m_vTextures[0]->SetDirty();
	}

	if (pRenderer->SetUseMaterial(m_pScreenDrawMaterial))
	{
		pRenderer->DrawMesh(m_pScreenDrawMesh);
	}

	pRenderer->EndRenderPass();
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

	AppendPostProcess<MGaussianBlurWork>();
}

void MForwardPostProcessProgram::Release()
{
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

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MRenderBackTexture* pBackTexture = new MRenderBackTexture();
		MRenderDepthTexture* pDepthTexture = new MRenderDepthTexture();

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
		m_pEngine->GetDevice()->DestroyRenderTarget(m_pTempRenderTarget);
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

