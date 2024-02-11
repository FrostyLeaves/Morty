#include "Render/SceneTexture.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "TaskGraph/MTaskGraph.h"
#include "Render/MRenderCommand.h"

#include "System/MObjectSystem.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MMoveControllerComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "RenderProgram/MIRenderProgram.h"

#include "stb_image_write.h"
#include "Batch/MMeshInstanceManager.h"
#include "Main/MainEditor.h"
#include "Shadow/MShadowMeshManager.h"
#include "Manager/MAnimationManager.h"

using namespace morty;

SceneTexture::SceneTexture()
	: m_bSnapshot(false)
	, m_strSnapshotPath("")
{

}

SceneTexture::~SceneTexture()
{

}

void SceneTexture::Initialize(MScene* pScene, const MString& strRenderProgram)
{
	m_pScene = pScene;

	MEngine* pEngine = pScene->GetEngine();
 	MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>();
	
	m_pRenderViewport = pObjectSystem->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2i(256, 256));

	MEntity* pDefaultCamera = m_pScene->CreateEntity();

	pDefaultCamera->SetName("Camera");
	if (MSceneComponent* pSceneComponent = pDefaultCamera->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(0, 20, 0));
		pSceneComponent->SetRotation(Quaternion(Vector3(1, 0, 0), 45.0f));
	}
	pDefaultCamera->RegisterComponent<MCameraComponent>();
	pDefaultCamera->RegisterComponent<MMoveControllerComponent>();

	m_pRenderViewport->SetCamera(pDefaultCamera);


	MObject* pRenderProgramObject = pObjectSystem->CreateObject(strRenderProgram);
	m_pRenderProgram = pRenderProgramObject->template DynamicCast<MIRenderProgram>();
	m_pRenderProgram->SetViewport(m_pRenderViewport);
	

	m_pUpdateTask = pEngine->GetMainGraph()->AddNode<MTaskNode>(MStringId("SceneTextureUpdate"));
	if (m_pUpdateTask)
	{
		m_pUpdateTask->SetThreadType(METhreadType::ERenderThread);

		GetScene()->GetManager<MMeshInstanceManager>()->GetUpdateTask()->ConnectTo(m_pUpdateTask);
		GetScene()->GetManager<MShadowMeshManager>()->GetUpdateTask()->ConnectTo(m_pUpdateTask);
		GetScene()->GetManager<MAnimationManager>()->GetUpdateTask()->ConnectTo(m_pUpdateTask);
	}
}

void SceneTexture::Release()
{
	MEngine* pEngine = m_pScene->GetEngine();

	if (m_pUpdateTask)
	{
		pEngine->GetMainGraph()->DestroyNode(m_pUpdateTask);
		m_pUpdateTask = nullptr;
	}

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

	m_pRenderProgram->DeleteLater();
	m_pRenderProgram = nullptr;
}

void SceneTexture::SetRect(Vector2i pos, Vector2i size)
{
	m_pRenderViewport->SetScreenPosition(pos);
	m_pRenderViewport->SetSize(size);
}

std::shared_ptr<MTexture> SceneTexture::GetTexture()
{
	return m_pRenderProgram->GetOutputTexture();
}

std::vector<std::shared_ptr<MTexture>> SceneTexture::GetAllOutputTexture()
{
	return m_pRenderProgram->GetOutputTextures();
}

void SceneTexture::Snapshot(const MString& strSnapshotPath)
{
	m_strSnapshotPath = strSnapshotPath;
	m_bSnapshot = true;
}

void SceneTexture::UpdateTexture(MIRenderCommand* pRenderCommand)
{
	if (m_bPauseUpdate)
	{
		return;
	}

	m_pRenderProgram->Render(pRenderCommand);

	if (m_bSnapshot)
	{
		pRenderCommand->DownloadTexture(GetTexture().get(), 0, [=](void* pImageData, const Vector2& v2Size) {
			stbi_write_png(m_strSnapshotPath.c_str(), v2Size.x, v2Size.y, 4, pImageData, v2Size.x * 4);
			});

		m_bSnapshot = false;
	}
}
