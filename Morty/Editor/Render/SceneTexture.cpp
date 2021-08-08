#include "SceneTexture.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MTaskGraph.h"
#include "MRenderTaskNode.h"

#include "MObjectSystem.h"
#include "MEntitySystem.h"
#include "MResourceSystem.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MoveInputNode.h"

#include "MForwardRenderProgram.h"

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
 
 	MObjectSystem* pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();
	m_pScene = pObjectSystem->CreateObject<MScene>();
	
	m_pRenderViewport = pObjectSystem->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2(256, 256));

	MEntity* pDefaultCamera = m_pScene->CreateEntity();

	pDefaultCamera->SetName("Camera");
	if (MSceneComponent* pSceneComponent = pDefaultCamera->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(0, 0, -20));
	}
	pDefaultCamera->RegisterComponent<MCameraComponent>();

	pDefaultCamera->RegisterComponent<MoveInputComponent>();

	m_pRenderViewport->SetCamera(pDefaultCamera);

	m_pRenderProgram = pObjectSystem->CreateObject<MForwardRenderProgram>();
	m_pRenderProgram->SetViewport(m_pRenderViewport);

	

	MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = m_pEngine->FindSystem<MEntitySystem>();
	
	MResource* pResource = pResourceSystem->LoadResource("D:/test/cat/cat.entity");
	pEntitySystem->LoadEntity(m_pScene, pResource);
	
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

	if (m_pRenderProgram)
	{
		m_pRenderProgram->DeleteLater();
		m_pRenderProgram = nullptr;
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

	if (m_pRenderViewport)
	{
		m_pRenderViewport->SetSize(m_v2Size);
	}
}

MTexture* SceneTexture::GetTexture()
{
	if (m_pRenderProgram)
	{
		return m_pRenderProgram->GetOutputTexture();
	}

	return nullptr;
}

void SceneTexture::SetBackColor(const MColor& cColor)
{
}

void SceneTexture::Render()
{
	if (m_pRenderProgram)
	{
		m_pRenderProgram->Render();
	}
}
