#include "SceneTexture.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MTaskGraph.h"
#include "MRenderTaskNode.h"

#include "MObjectSystem.h"
#include "MSceneComponent.h"

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
// 	m_pEngine = pEngine;
// 
// 	MObjectSystem* pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();
// 
// 	m_pRenderProgram = pObjectSystem->CreateObject<MForwardRenderProgram>();
// 	//m_pRenderProgram = m_pEngine->GetObjectManager()->CreateObject<MDeferredRenderProgram>();
// 
// 	m_pScene = pObjectSystem->CreateObject<MScene>();
// 
// 	m_pRenderViewport = pObjectSystem->CreateObject<MViewport>();
// 	m_pRenderViewport->SetScene(m_pScene);
// 	m_pRenderViewport->SetSize(Vector2(256, 256));
// 
// 	m_pRenderProgram->Initialize();
// 
// 	MEntity* pCameraNode = m_pRenderViewport->GetCamera();
// 	
// 	if (MSceneComponent* pSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
// 	{
// 		pSceneComponent->SetPosition(Vector3(0, 0, -20));
// 	}
}

void SceneTexture::Release()
{
	if (m_pScene)
	{
		m_pScene->DeleteLater();
		m_pScene = nullptr;
	}

	if (m_pRenderViewport)
	{
		m_pRenderViewport->DeleteLater();
		m_pRenderViewport = nullptr;
	}

// 	m_pRenderProgram->DeleteLater();
// 	m_pRenderProgram = nullptr;

}

void SceneTexture::SetSize(const Vector2& v2Size)
{
// 	if (m_v2Size == v2Size)
// 		return;
// 
// 	m_v2Size = v2Size;
// 
// 	if (m_v2Size.x < 1.0f)
// 		m_v2Size.x = 1.0f;
// 
// 	if (m_v2Size.y < 1.0f)
// 		m_v2Size.y = 1.0f;
// 
// 	if (MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph())
// 	{
// 		pRenderGraph->SetOutputSize(m_v2Size);
// 	}
// 
// 	if (m_pRenderViewport)
// 	{
// 		m_pRenderViewport->SetSize(m_v2Size);
// 	}
}

void SceneTexture::UpdateTexture(MIRenderCommand* pCommand)
{
// 	if (m_pRenderProgram)
// 	{
// 		m_pRenderProgram->Render(m_pEngine->GetRenderer(), m_pRenderViewport, pCommand);
// 	}
}

MTexture* SceneTexture::GetTexture()
{
	return nullptr;
}

void SceneTexture::SetBackColor(const MColor& cColor)
{
}
