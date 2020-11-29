#include "MForwardPostProcessProgram.h"

#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "MForwardHDRWork.h"
#include "MForwardRenderWork.h"
#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"

M_OBJECT_IMPLEMENT(MForwardPostProcessProgram, MForwardRenderProgram)

MForwardPostProcessProgram::MForwardPostProcessProgram()
    : MForwardRenderProgram()
	, m_pRenderHDRWork(nullptr)
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

	info.unFrameIndex = pRenderer->GetFrameIndex();
	info.pRenderTarget = GetRenderTarget();
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCamera = pViewport->GetCamera();
	info.pScene = pViewport->GetScene();

	if (m_pRenderHDRWork)
	{
		m_pRenderHDRWork->CheckTextureSize(info);
		info.pRenderTarget = m_pRenderHDRWork->GetTempRenderTarget();
	}

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

	if (m_pRenderHDRWork)
	{
		m_pRenderHDRWork->Render(info);
	}

	pViewport->UnlockMatrix();
}

void MForwardPostProcessProgram::Initialize()
{
    Super::Initialize();

	m_pRenderHDRWork = GetEngine()->GetObjectManager()->CreateObject<MForwardHDRWork>();
	m_pRenderHDRWork->Initialize(this);
}

void MForwardPostProcessProgram::Release()
{
	if (m_pRenderHDRWork)
	{
		m_pRenderHDRWork->DeleteLater();
		m_pRenderHDRWork = nullptr;
	}

    Super::Release();
}

