#include "MDeferredRenderProgram.h"
#include "Utility/MGlobal.h"
#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "RHI/IRenderCommand.h"
#include "Render/RenderGraph/MRenderGraphWalker.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MMultiThreadTaskGraphWalker.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MBounds.h"
#include "Flatbuffer/MRenderGraph_generated.h"
#include "Flatbuffer/MTaskNode_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)

const MStringId FinalBackBuffer = MStringId("Final Back Buffer");

void            MDeferredRenderProgram::Render(IRenderCommand* pPrimaryCommand)
{
    if (!GetViewport()) return;
    if (m_renderGraph->NeedCompile()) { m_renderGraph->Compile(); }
    if (m_renderGraph->GetFinalNodes().empty()) { return; }

    RenderSetup(pPrimaryCommand);

    //Run Render Graph
    MRenderGraphWalker walker(m_renderInfo);
    walker(m_renderGraph.get());
}

void MDeferredRenderProgram::RenderSetup(IRenderCommand* pPrimaryCommand)
{

    MViewport* pViewport = GetViewport();
    //MEntity*   pCameraEntity         = pViewport->GetCamera();
    //MScene*    scene                = pViewport->GetScene();
    //MEntity*   pMainDirectionalLight = scene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    //auto*      pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();

    m_renderInfo                       = MRenderInfo::CreateFromViewport(pViewport);
    m_renderInfo.nFrameIndex           = m_frameIndex++;
    m_renderInfo.pPrimaryRenderCommand = pPrimaryCommand;

    //Culling Task.
    //MMultiThreadTaskGraphWalker walker(GetEngine()->GetThreadPool());
    //walker(m_cullingTask.get());

    //Resize FrameBuffer.
    const Vector2i v2ViewportSize = pViewport->GetSize();
    m_renderGraph->Resize(v2ViewportSize);

    MRenderGraphSetupWalker setupWalker(m_renderInfo);
    setupWalker(m_renderGraph.get());
}

void MDeferredRenderProgram::OnCreated()
{
    Super::OnCreated();

    InitializeTaskGraph();
    InitializeFrameShaderParams();
    InitializeRenderGraph();
}

void MDeferredRenderProgram::OnDelete()
{
    Super::OnDelete();

    ReleaseTaskGraph();
    ReleaseFrameShaderParams();
    ReleaseRenderGraph();
}

void MDeferredRenderProgram::InitializeTaskGraph() {}

void MDeferredRenderProgram::ReleaseTaskGraph() { m_renderGraph = nullptr; }

void MDeferredRenderProgram::InitializeRenderGraph() { m_renderGraph = std::make_unique<MRenderGraph>(GetEngine()); }

void MDeferredRenderProgram::ReleaseRenderGraph() { m_renderGraph = nullptr; }

void MDeferredRenderProgram::InitializeFrameShaderParams() {}

void MDeferredRenderProgram::ReleaseFrameShaderParams() {}

void MDeferredRenderProgram::LoadGraph(const std::vector<MByte>& buffer)
{
    ReleaseRenderGraph();
    ReleaseFrameShaderParams();

    InitializeFrameShaderParams();
    InitializeRenderGraph();


    flatbuffers::FlatBufferBuilder fbb;
    fbb.PushBytes((const uint8_t*) buffer.data(), buffer.size());
    const fbs::MRenderGraph* fbRenderGraph = fbs::GetMRenderGraph(fbb.GetCurrentBufferPointer());

    m_renderGraph->Deserialize(fbRenderGraph);
}

void MDeferredRenderProgram::SaveGraph(std::vector<MByte>& output)
{
    output.clear();

    flatbuffers::FlatBufferBuilder fbb;
    auto                           allTaskNode = m_renderGraph->GetAllNodes();

    fbb.Finish(m_renderGraph->Serialize(fbb));

    output.resize(fbb.GetSize());
    memcpy(output.data(), fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));
}
