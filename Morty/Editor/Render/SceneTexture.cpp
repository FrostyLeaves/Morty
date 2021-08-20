#include "SceneTexture.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MTaskGraph.h"

#include "MObjectSystem.h"
#include "MEntitySystem.h"
#include "MResourceSystem.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MMoveControllerComponent.h"
#include "MDirectionalLightComponent.h"

#include "MForwardRenderProgram.h"

SceneTexture::SceneTexture()
	: m_pEngine(nullptr)
	, m_pScene(nullptr)
	, m_pRenderViewport(nullptr)
	, m_vRenderProgram()
	, m_nImageCount(3)
{

}

SceneTexture::~SceneTexture()
{

}

void SceneTexture::Initialize(MEngine* pEngine, const size_t& nImageCount)
{
 	m_pEngine = pEngine;
	m_nImageCount = nImageCount;
 
 	MObjectSystem* pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();
	m_pScene = pObjectSystem->CreateObject<MScene>();
	
	m_pRenderViewport = pObjectSystem->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2(256, 256));

	MEntity* pDefaultCamera = m_pScene->CreateEntity();

	pDefaultCamera->SetName("Camera");
	if (MSceneComponent* pSceneComponent = pDefaultCamera->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(0, 0, -75));
	}
	pDefaultCamera->RegisterComponent<MCameraComponent>();
	pDefaultCamera->RegisterComponent<MMoveControllerComponent>();

	m_pRenderViewport->SetCamera(pDefaultCamera);

	m_vRenderProgram.resize(nImageCount);
	for (size_t i = 0; i < nImageCount; ++i)
	{
		m_vRenderProgram[i] = pObjectSystem->CreateObject<MForwardRenderProgram>();
		m_vRenderProgram[i]->SetViewport(m_pRenderViewport);
	}

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

	for (MIRenderProgram* pRenderProgram : m_vRenderProgram)
	{
		pRenderProgram->DeleteLater();
		pRenderProgram = nullptr;
	}
	m_vRenderProgram.clear();
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

MTexture* SceneTexture::GetTexture(const size_t& nImageIndex)
{
	if (nImageIndex < m_vRenderProgram.size())
	{
		return m_vRenderProgram[nImageIndex]->GetOutputTexture();
	}

	return nullptr;
}

void SceneTexture::SetBackColor(const MColor& cColor)
{
}

void SceneTexture::UpdateTexture(const size_t& nImageIndex, MIRenderCommand* pRenderCommand)
{
	if (nImageIndex < m_vRenderProgram.size())
	{
		m_vRenderProgram[nImageIndex]->Render(pRenderCommand);
	}
}
