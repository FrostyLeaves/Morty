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

SceneTexture::SceneTexture()
	: m_pEngine(nullptr)
	, m_pScene(nullptr)
	, m_pTextureRenderTarget(nullptr)
	, m_pRenderViewport(nullptr)
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
	m_pTextureRenderTarget = MTextureRenderTarget::CreateForTexture(m_pEngine->GetDevice(), MTextureRenderTarget::ERenderBack | MTextureRenderTarget::ERenderDepth, 256, 256);
	m_pTextureRenderTarget->m_backgroundColor = MColor(0, 0, 0, 1);
	m_pTextureRenderTarget->m_funcRenderFunction = [this](MIRenderer* pRenderer)
	{
		m_pRenderViewport->Render(pRenderer);
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

	m_pTextureRenderTarget->Release(m_pEngine->GetDevice());
	delete m_pTextureRenderTarget;
	m_pTextureRenderTarget = nullptr;
}

void SceneTexture::SetSize(const Vector2& v2Size)
{
	m_v2Size = v2Size;

	m_pTextureRenderTarget->OnResize(m_v2Size.x, m_v2Size.y);
	m_pRenderViewport->SetSize(Vector2(m_v2Size.x, m_v2Size.y));
}

void SceneTexture::UpdateTexture()
{
	m_pEngine->GetRenderer()->Render(m_pTextureRenderTarget);
}

void* SceneTexture::GetTexture()
{
	if (m_pTextureRenderTarget)
	{
		if (m_pTextureRenderTarget)
		{
			if (m_pTextureRenderTarget->m_pBackTexture)
			{
				if (MTextureBuffer* pBuffer = m_pTextureRenderTarget->m_pBackTexture->GetBuffer())
				{
					return pBuffer->GetResourceView();
				}
			}
		}
	}

	return nullptr;
}
