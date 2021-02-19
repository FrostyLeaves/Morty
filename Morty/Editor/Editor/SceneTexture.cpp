#include "SceneTexture.h"

#include "MScene.h"
#include "MCamera.h"
#include "M3DNode.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTexture.h"
#include "MRenderStructure.h"
#include "MForwardRenderProgram.h"
#include "MForwardPostProcessProgram.h"

SceneTexture::SceneTexture()
	: m_pEngine(nullptr)
	, m_pScene(nullptr)
	, m_pRenderViewport(nullptr)
	, m_pRenderProgram(nullptr)
{

}

SceneTexture::~SceneTexture()
{

}

void SceneTexture::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	//m_pRenderProgram = m_pEngine->GetObjectManager()->CreateObject<MForwardRenderProgram>();
	m_pRenderProgram = m_pEngine->GetObjectManager()->CreateObject<MForwardPostProcessProgram>();

	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MScene>();

	m_pRenderViewport = pEngine->GetObjectManager()->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2(256, 256));

	m_pRenderProgram->Initialize();

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

	m_pRenderProgram->DeleteLater();
	m_pRenderProgram = nullptr;

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

	if (MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph())
	{
		if (MRenderGraphTexture* pRenderGraphTexture = pRenderGraph->GetFinalOutputTexture())
		{
			m_pRenderViewport->SetSize(pRenderGraphTexture->GetSize());
		}
	}

}

void SceneTexture::UpdateTexture()
{
	if (m_pRenderProgram)
	{
		m_pRenderProgram->Render(m_pEngine->GetRenderer(), m_pRenderViewport);
	}
}

void* SceneTexture::GetTexture(const uint32_t& unFrameIndex)
{

	if (MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph())
	{
		if (MRenderGraphTexture* pRenderGraphTexture = pRenderGraph->GetFinalOutputTexture())
		{
			if (MIRenderTexture* pTexture = pRenderGraphTexture->GetRenderTexture())
			{
				return pTexture;
			}
		}
	}

	return nullptr;
}

void SceneTexture::SetBackColor(const MColor& cColor)
{
	m_pRenderProgram->SetClearColor(cColor);
}

MRenderGraph* SceneTexture::GetRenderGraph()
{
	if (m_pRenderProgram)
	{
		return m_pRenderProgram->GetRenderGraph();
	}

	return nullptr;
}
