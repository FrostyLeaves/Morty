#include "Render/SceneTexture.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "RHI/MRenderCommand.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "TaskGraph/MTaskGraph.h"

#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MMoveControllerComponent.h"
#include "Component/MSceneComponent.h"

#include "RenderProgram/MIRenderProgram.h"

#include "Batch/MMeshInstanceManager.h"
#include "Main/MainEditor.h"
#include "Manager/MAnimationManager.h"
#include "Shadow/MShadowMeshManager.h"
#include "stb_image_write.h"

using namespace morty;

SceneTexture::SceneTexture()
    : m_snapshot(false)
    , m_strSnapshotPath("")
{}

SceneTexture::~SceneTexture() {}

void SceneTexture::Initialize(MScene* pScene, const MString& strRenderProgram)
{
    m_scene = pScene;

    MEngine*       pEngine       = pScene->GetEngine();
    MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>();

    m_renderViewport = pObjectSystem->CreateObject<MViewport>();
    m_renderViewport->SetScene(m_scene);
    m_renderViewport->SetSize(Vector2i(256, 256));

    MEntity* pDefaultCamera = m_scene->CreateEntity();

    pDefaultCamera->SetName("Camera");
    if (MSceneComponent* pSceneComponent =
                pDefaultCamera->RegisterComponent<MSceneComponent>())
    {
        pSceneComponent->SetPosition(Vector3(0, 20, 0));
        pSceneComponent->SetRotation(Quaternion(Vector3(1, 0, 0), 45.0f));
    }
    pDefaultCamera->RegisterComponent<MCameraComponent>();
    pDefaultCamera->RegisterComponent<MMoveControllerComponent>();

    m_renderViewport->SetCamera(pDefaultCamera);


    MObject* pRenderProgramObject = pObjectSystem->CreateObject(strRenderProgram);
    m_renderProgram = pRenderProgramObject->template DynamicCast<MIRenderProgram>();
    m_renderProgram->SetViewport(m_renderViewport);


    m_updateTask =
            pEngine->GetMainGraph()->AddNode<MTaskNode>(MStringId("SceneTextureUpdate"));
    if (m_updateTask)
    {
        m_updateTask->SetThreadType(METhreadType::ERenderThread);

        GetScene()->GetManager<MMeshInstanceManager>()->GetUpdateTask()->ConnectTo(
                m_updateTask
        );
        GetScene()->GetManager<MShadowMeshManager>()->GetUpdateTask()->ConnectTo(
                m_updateTask
        );
        GetScene()->GetManager<MAnimationManager>()->GetUpdateTask()->ConnectTo(
                m_updateTask
        );
    }
}

void SceneTexture::Release()
{
    MEngine* pEngine = m_scene->GetEngine();

    if (m_updateTask)
    {
        pEngine->GetMainGraph()->DestroyNode(m_updateTask);
        m_updateTask = nullptr;
    }

    if (m_scene)
    {
        m_scene->DeleteLater();
        m_scene = nullptr;
    }

    if (m_renderViewport)
    {
        m_renderViewport->DeleteLater();
        m_renderViewport = nullptr;
    }

    m_renderProgram->DeleteLater();
    m_renderProgram = nullptr;
}

void SceneTexture::SetRect(Vector2i pos, Vector2i size)
{
    m_renderViewport->SetScreenPosition(pos);
    m_renderViewport->SetSize(size);
}

std::shared_ptr<MTexture> SceneTexture::GetTexture()
{
    return m_renderProgram->GetOutputTexture();
}

std::vector<std::shared_ptr<MTexture>> SceneTexture::GetAllOutputTexture()
{
    return m_renderProgram->GetOutputTextures();
}

void SceneTexture::Snapshot(const MString& strSnapshotPath)
{
    m_strSnapshotPath = strSnapshotPath;
    m_snapshot        = true;
}

void SceneTexture::UpdateTexture(MIRenderCommand* pRenderCommand)
{
    if (m_pauseUpdate) { return; }

    m_renderProgram->Render(pRenderCommand);

    if (m_snapshot)
    {
        pRenderCommand->DownloadTexture(
                GetTexture().get(),
                0,
                [=](void* pImageData, const Vector2& v2Size) {
                    stbi_write_png(
                            m_strSnapshotPath.c_str(),
                            v2Size.x,
                            v2Size.y,
                            4,
                            pImageData,
                            v2Size.x * 4
                    );
                }
        );

        m_snapshot = false;
    }
}
