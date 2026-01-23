#include "Render/SceneViewer.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MMoveControllerComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "RHI/IRenderCommand.h"
#include "Render/MIRenderProgram.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "stb_image_write.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(SceneViewer, MObject)

MString SceneViewer::m_defaultRenderGraphPath = MString(MORTY_RESOURCE_PATH) + "/Pipeline/default_render_graph.mrg";

void    SceneViewer::Initialize(const MString& viewName, MScene* scene, const MStringId& strRenderProgram)
{
    m_scene = scene;

    MEngine* engine       = GetEngine();
    auto*    objectSystem = engine->GetSystem<MObjectSystem>();

    m_renderViewport = objectSystem->CreateObject<MViewport>();
    m_renderViewport->SetScene(m_scene);
    m_renderViewport->SetSize(Vector2i(256, 256));

    MEntity* defaultCamera = m_scene->CreateEntity();

    defaultCamera->SetName("Camera");
    if (auto* sceneComponent = defaultCamera->RegisterComponent<MSceneComponent>())
    {
        //sceneComponent->SetPosition(Vector3(0, 20, 0));
        //sceneComponent->SetRotation(Quaternion(Vector3(1, 0, 0), 45.0f));
        sceneComponent->SetPosition(Vector3(0, 0, 0));
    }
    defaultCamera->RegisterComponent<MCameraComponent>();
    defaultCamera->RegisterComponent<MMoveControllerComponent>();

    m_renderViewport->SetCamera(defaultCamera);


    MObject* pRenderProgramObject = objectSystem->CreateObject(strRenderProgram);
    m_renderProgram               = pRenderProgramObject->template DynamicCast<MIRenderProgram>();
    m_renderProgram->SetViewport(m_renderViewport);

    std::vector<MByte> renderGraphBuffer;
    MORTY_ASSERT(MFileHelper::ReadData(m_defaultRenderGraphPath, renderGraphBuffer));
    m_renderProgram->LoadGraph(renderGraphBuffer);

    m_updateTask = engine->GetMainGraph()->AddNode<MTaskNode>(MStringId("SceneView_" + viewName));
    if (m_updateTask) { m_updateTask->SetThreadType(METhreadType::ERenderThread); }
}

void SceneViewer::OnDelete()
{
    MEngine* engine = GetEngine();

    if (m_updateTask)
    {
        engine->GetMainGraph()->DestroyNode(m_updateTask);
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

void SceneViewer::Tick(float fDelta)
{
    MORTY_UNUSED(fDelta);
    m_renderProgram->Update();
}

void SceneViewer::Render(IRenderCommand* pRenderCommand)
{
    if (m_pauseUpdate) { return; }

    m_renderProgram->Render(pRenderCommand);
}

MTexturePtr SceneViewer::GetFinalOutputTexture() const
{
    auto pGraph = m_renderProgram->GetRenderGraph();
    return pGraph->GetFinalOutput();
}
