#include "Render/SceneViewer.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Batch/MMeshInstanceManager.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MMoveControllerComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "Manager/MAnimationManager.h"
#include "RHI/MRenderCommand.h"
#include "Render/MIRenderProgram.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Shadow/MShadowMeshManager.h"
#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "stb_image_write.h"

using namespace morty;

MString SceneViewer::m_defaultRenderGraphPath = MString(MORTY_RESOURCE_PATH) + "/Pipeline/default_render_graph.mrg";

void    SceneViewer::Initialize(const MString& viewName, MScene* pScene, const MString& strRenderProgram)
{
    m_scene = pScene;

    MEngine* pEngine       = pScene->GetEngine();
    auto*    pObjectSystem = pEngine->FindSystem<MObjectSystem>();

    m_renderViewport = pObjectSystem->CreateObject<MViewport>();
    m_renderViewport->SetScene(m_scene);
    m_renderViewport->SetSize(Vector2i(256, 256));

    MEntity* pDefaultCamera = m_scene->CreateEntity();

    pDefaultCamera->SetName("Camera");
    if (auto* pSceneComponent = pDefaultCamera->RegisterComponent<MSceneComponent>())
    {
        pSceneComponent->SetPosition(Vector3(0, 20, 0));
        pSceneComponent->SetRotation(Quaternion(Vector3(1, 0, 0), 45.0f));
    }
    pDefaultCamera->RegisterComponent<MCameraComponent>();
    pDefaultCamera->RegisterComponent<MMoveControllerComponent>();

    m_renderViewport->SetCamera(pDefaultCamera);


    MObject* pRenderProgramObject = pObjectSystem->CreateObject(strRenderProgram);
    m_renderProgram               = pRenderProgramObject->template DynamicCast<MIRenderProgram>();
    m_renderProgram->SetViewport(m_renderViewport);

    std::vector<MByte> renderGraphBuffer;
    MORTY_ASSERT(MFileHelper::ReadData(m_defaultRenderGraphPath, renderGraphBuffer));
    m_renderProgram->LoadGraph(renderGraphBuffer);

    m_updateTask = pEngine->GetMainGraph()->AddNode<MTaskNode>(MStringId("SceneView_" + viewName));
    if (m_updateTask)
    {
        m_updateTask->SetThreadType(METhreadType::ERenderThread);

        GetScene()->GetManager<MMeshInstanceManager>()->GetUpdateTask()->ConnectTo(m_updateTask);
        GetScene()->GetManager<MShadowMeshManager>()->GetUpdateTask()->ConnectTo(m_updateTask);
        GetScene()->GetManager<MAnimationManager>()->GetUpdateTask()->ConnectTo(m_updateTask);
    }
}

void SceneViewer::Release()
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

void SceneViewer::SetRect(Vector2i pos, Vector2i size)
{
    m_renderViewport->SetScreenPosition(pos);
    m_renderViewport->SetSize(size);
}

void SceneViewer::UpdateTexture(MIRenderCommand* pRenderCommand)
{
    if (m_pauseUpdate) { return; }

    m_renderProgram->Render(pRenderCommand);
}
void SceneViewer::SetFinalOutput(const size_t nodeId, const size_t nSlotId)
{
    m_finalOutputNode = nodeId;
    m_finalOutputSlot = nSlotId;
}

MTexturePtr SceneViewer::GetFinalOutputTexture() const
{
    auto pNode = m_renderProgram->GetRenderGraph()->FindRenderNode(m_finalOutputNode);
    if (pNode == nullptr) { return nullptr; }

    auto pOutput = pNode->GetRenderOutput(m_finalOutputSlot);
    if (pOutput == nullptr) { return nullptr; }

    return pOutput->GetRenderTexture();
}
