#include "MStandardPostProcessWork.h"
#include "MEngine.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTextureRenderTarget.h"

#include "MResourceManager.h"
#include "Model/MMeshResource.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MStandardPostProcessWork, MIPostProcessWork)

MStandardPostProcessWork::MStandardPostProcessWork()
	: MIPostProcessWork()
	, m_pRenderProgram(nullptr)
	, m_pTempRenderTarget(nullptr)
	, m_pTempRenderPass(nullptr)
	, m_pScreenDrawMesh(nullptr)
	, m_aBackTexture()
	, m_aDepthTexture()
{
}

MStandardPostProcessWork::~MStandardPostProcessWork()
{
}

void MStandardPostProcessWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeMesh();
	InitializeRenderTargets();
	InitializeRenderPass();
}

void MStandardPostProcessWork::Release()
{
	ReleaseRenderPass();
	ReleaseRenderTargets();
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

void MStandardPostProcessWork::InitializeRenderTargets()
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

void MStandardPostProcessWork::ReleaseRenderTargets()
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

void MStandardPostProcessWork::InitializeRenderPass()
{
	if (!m_pTempRenderTarget)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderPass error: rt == nullptr");
		return;
	}

	//Init RenderPass
	m_pTempRenderPass = new MRenderPass();
	m_pTempRenderPass->m_vBackDesc.push_back(MPassTargetDescription());
	m_pTempRenderPass->m_vBackDesc.back().bClearWhenRender = true;
	m_pTempRenderPass->m_vBackDesc.back().cClearColor = m_pRenderProgram->GetClearColor();

	m_pTempRenderPass->m_DepthDesc.bClearWhenRender = true;
}

void MStandardPostProcessWork::ReleaseRenderPass()
{
	if (m_pTempRenderPass)
	{
		GetEngine()->GetDevice()->DestroyRenderPass(m_pTempRenderPass);
		delete m_pTempRenderPass;
		m_pTempRenderPass = nullptr;
	}
}

void MStandardPostProcessWork::CheckRenderTargetSize(const Vector2& v2ViewportSize)
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

void MStandardPostProcessWork::Render(MPostProcessRenderInfo& info)
{

}

MTextureRenderTarget* MStandardPostProcessWork::GetRenderTarget()
{
	return m_pTempRenderTarget;
}
