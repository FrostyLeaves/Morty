#include "MRenderGraphProgram.h"
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

MORTY_CLASS_IMPLEMENT(MRenderGraphProgram, MIRenderProgram)

const MStringId FinalBackBuffer = MStringId("Final Back Buffer");


void            MRenderGraphProgram::Update()
{
    MViewport* viewport      = GetViewport();
    m_renderInfo             = MRenderInfo::CreateFromViewport(viewport);
    m_renderInfo.nFrameIndex = m_frameIndex++;
}

void MRenderGraphProgram::Render(IRenderCommand* primaryCommand)
{
    if (!GetViewport()) return;
    if (m_renderGraph->NeedCompile()) { m_renderGraph->Compile(); }
    if (m_renderGraph->GetFinalNodes().empty()) { return; }

    //Resize FrameBuffer.
    m_renderGraph->Resize(m_renderInfo.viewportRect.GetSize());

    MRenderGraphSetupWalker setupWalker(m_renderInfo);
    setupWalker(m_renderGraph.get());

    //Run Render Graph
    MRenderGraphWalker walker(m_renderInfo, primaryCommand);
    walker(m_renderGraph.get());
}

void MRenderGraphProgram::OnCreated()
{
    Super::OnCreated();

    InitializeTaskGraph();
    InitializeFrameShaderParams();
    InitializeRenderGraph();
}

void MRenderGraphProgram::OnDelete()
{
    Super::OnDelete();

    ReleaseTaskGraph();
    ReleaseFrameShaderParams();
    ReleaseRenderGraph();
}

void MRenderGraphProgram::InitializeTaskGraph() {}

void MRenderGraphProgram::ReleaseTaskGraph() { m_renderGraph = nullptr; }

void MRenderGraphProgram::InitializeRenderGraph() { m_renderGraph = std::make_unique<MRenderGraph>(GetEngine()); }

void MRenderGraphProgram::ReleaseRenderGraph() { m_renderGraph = nullptr; }

void MRenderGraphProgram::InitializeFrameShaderParams() {}

void MRenderGraphProgram::ReleaseFrameShaderParams() {}

void MRenderGraphProgram::LoadGraph(const std::vector<MByte>& buffer)
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

void MRenderGraphProgram::SaveGraph(std::vector<MByte>& output)
{
    output.clear();

    flatbuffers::FlatBufferBuilder fbb;
    auto                           allTaskNode = m_renderGraph->GetAllNodes();

    fbb.Finish(m_renderGraph->Serialize(fbb));

    output.resize(fbb.GetSize());
    memcpy(output.data(), fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));
}
