#include "SceneTexture.h"

#include "MScene.h"
#include "MCamera.h"
#include "M3DNode.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MRenderStructure.h"
#include "MTextureRenderTarget.h"
#include "MForwardRenderProgram.h"

SceneTexture::SceneTexture()
	: m_pEngine(nullptr)
	, m_pScene(nullptr)
	, m_pTextureRenderTarget(nullptr)
	, m_pRenderViewport(nullptr)
	, m_vBackTexture()
	, m_vDepthTexture()
	, m_pRenderProgram(nullptr)
{

}

SceneTexture::~SceneTexture()
{

}

void SceneTexture::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_pRenderProgram = m_pEngine->GetObjectManager()->CreateObject<MForwardRenderProgram>();

	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MScene>();

	m_pRenderViewport = pEngine->GetObjectManager()->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2(256, 256));

	
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MRenderBackTexture* pBackTexture = new MRenderBackTexture();
		pBackTexture->SetSize(Vector2(256, 256));
		pBackTexture->SetType(METextureLayout::ERGBA8);
		pBackTexture->GenerateBuffer(pEngine->GetDevice());

		m_vBackTexture[i] = pBackTexture;

		MRenderDepthTexture* pDepthTexture = new MRenderDepthTexture();
		pDepthTexture->SetSize(Vector2(256, 256));
		pDepthTexture->GenerateBuffer(pEngine->GetDevice());

		m_vDepthTexture[i] = pDepthTexture;
	}
	m_pTextureRenderTarget = pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();
	m_pTextureRenderTarget->SetBackTexture(m_vBackTexture, 0);
	m_pTextureRenderTarget->SetDepthTexture(m_vDepthTexture);
	m_pTextureRenderTarget->Resize({ 256, 256 });

	m_pRenderProgram->BindRenderTarget(m_pTextureRenderTarget);

	MCamera* pCamera = m_pRenderViewport->GetCamera();
	pCamera->SetPosition(Vector3(0, 0, -20));

}

void SceneTexture::Release()
{
	MObjectManager* pObjManager = m_pEngine->GetObjectManager();

	m_pScene->DeleteLater();
	m_pScene = nullptr;

	m_pRenderViewport->DeleteLater();
	m_pRenderViewport = nullptr;

	m_pEngine->GetDevice()->DestroyRenderTarget(m_pTextureRenderTarget);

	m_pTextureRenderTarget->DeleteLater();
	m_pTextureRenderTarget = nullptr;

	m_pRenderProgram->DeleteLater();
	m_pRenderProgram = nullptr;

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_vBackTexture[i]->DestroyBuffer(m_pEngine->GetDevice());
		m_vDepthTexture[i]->DestroyBuffer(m_pEngine->GetDevice());

		m_vBackTexture[i] = nullptr;
		m_vDepthTexture[i] = nullptr;
	}
}

void SceneTexture::SetSize(const Vector2& v2Size)
{
	if (m_v2Size == v2Size)
		return;

	m_v2Size = v2Size;

	if (m_v2Size.x < 1.0f)
		m_v2Size.x = 1.0f;

	if (m_v2Size.y < 1.0f)
		m_v2Size.y = 1.0f;

	m_pRenderViewport->SetSize(m_v2Size);

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_vBackTexture[i]->SetSize(m_v2Size);
		m_vDepthTexture[i]->SetSize(m_v2Size);

		m_vBackTexture[i]->DestroyBuffer(m_pEngine->GetDevice());
		m_vBackTexture[i]->GenerateBuffer(m_pEngine->GetDevice());

		m_vDepthTexture[i]->DestroyBuffer(m_pEngine->GetDevice());
		m_vDepthTexture[i]->GenerateBuffer(m_pEngine->GetDevice());
	}

	m_pTextureRenderTarget->Resize(m_v2Size);
}

void SceneTexture::UpdateTexture()
{
	if (m_pRenderProgram)
	{
		std::vector<MViewport*> vViewports = { m_pRenderViewport };
		m_pRenderProgram->Render(m_pEngine->GetRenderer(), vViewports);
	}
}

void* SceneTexture::GetTexture(const uint32_t& unFrameIndex)
{
	if (m_pTextureRenderTarget)
	{
		std::vector<MIRenderBackTexture*>*  pBackTextures = m_pTextureRenderTarget->GetBackTexture(unFrameIndex);
		if (MIRenderBackTexture* pBackTexture = pBackTextures->at(0))
		{
			if (MTextureBuffer* pBuffer = pBackTexture->GetBuffer())
			{
				return pBuffer->GetResourceView();
			}
		}
	}
	
	return nullptr;
}

void SceneTexture::SetBackColor(const MColor& cColor)
{
	m_pRenderProgram->SetClearColor(cColor);
}
