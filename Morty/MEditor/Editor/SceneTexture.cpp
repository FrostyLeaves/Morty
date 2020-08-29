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
	, m_pBackTexture(nullptr)
	, m_pDepthTexture(nullptr)
{

}

SceneTexture::~SceneTexture()
{

}

void SceneTexture::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MScene>();

	M3DNode* pRootNode = m_pEngine->GetObjectManager()->CreateObject<M3DNode>();
	m_pScene->SetRootNode(pRootNode);


	m_pRenderViewport = pEngine->GetObjectManager()->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2(256, 256));
	m_pRenderViewport->RegisterRenderProgram<MForwardRenderProgram>();


	m_pBackTexture = new MRenderBackTexture();
	m_pBackTexture->SetSize(Vector2(256, 256));
	m_pBackTexture->SetType(METextureLayout::ERGBA8);
	m_pBackTexture->GenerateBuffer(pEngine->GetDevice());

	m_pDepthTexture = new MRenderDepthTexture();
	m_pDepthTexture->SetSize(Vector2(256, 256));
	m_pDepthTexture->GenerateBuffer(pEngine->GetDevice());

	m_pTextureRenderTarget = pEngine->GetObjectManager()->CreateObject<MTextureRenderTarget>();
	m_pTextureRenderTarget->SetBackTexture(m_pBackTexture, 0, true, MColor::Black);
	m_pTextureRenderTarget->SetDepthTexture(m_pDepthTexture, true);

	m_pTextureRenderTarget->m_funcRenderFunction = [this](MIRenderer* pRenderer)
	{
		m_pRenderViewport->Render(pRenderer, m_pTextureRenderTarget);
	};


	MCamera* pCamera = m_pRenderViewport->GetCamera();
	pCamera->SetPosition(Vector3(0, 0, -20));

}

void SceneTexture::Release()
{
	//TODO Release RenderTarget and Viewport
	MObjectManager* pObjManager = m_pEngine->GetObjectManager();

	m_pScene->DeleteLater();
	m_pScene = nullptr;

	m_pRenderViewport->DeleteLater();
	m_pRenderViewport = nullptr;

	m_pTextureRenderTarget->DeleteLater();
	m_pTextureRenderTarget = nullptr;


	m_pBackTexture->DestroyTexture(m_pEngine->GetDevice());
	m_pDepthTexture->DestroyTexture(m_pEngine->GetDevice());
}

void SceneTexture::SetSize(const Vector2& v2Size)
{
	if (m_v2Size == v2Size)
		return;

	m_v2Size = v2Size;

	m_pRenderViewport->SetSize(m_v2Size);
	m_pBackTexture->SetSize(m_v2Size);
	m_pDepthTexture->SetSize(m_v2Size);

	m_pBackTexture->DestroyTexture(m_pEngine->GetDevice());
	m_pBackTexture->GenerateBuffer(m_pEngine->GetDevice());

	m_pDepthTexture->DestroyTexture(m_pEngine->GetDevice());
	m_pDepthTexture->GenerateBuffer(m_pEngine->GetDevice());
}

void SceneTexture::UpdateTexture()
{
	m_pEngine->GetRenderer()->Render(m_pTextureRenderTarget);
}

void* SceneTexture::GetTexture()
{
	if (m_pTextureRenderTarget)
	{
		if (MRenderBackTexture* pBackTexture = m_pTextureRenderTarget->GetBackTexture(0))
		{
			if (MTextureBuffer* pBuffer = pBackTexture->GetBuffer())
			{
				return pBuffer->GetResourceView();
			}
		}
	}
	
	return nullptr;
}
