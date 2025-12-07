#include "MSceneCullingNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Batch/Mesh/MMeshInstanceManager.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSceneCullingNode, MRenderTaskNode)

void MSceneCullingNode::OnCreated()
{
    Super::OnCreated();

    auto objectSystem = GetEngine()->GetSystem<MObjectSystem>();

    m_cullingDispatcher = objectSystem->CreateObject<MComputeDispatcher>();
    //m_cullingDispatcher->LoadComputeShader()
}

VOID MSceneCullingNode::OnDelete()
{
    m_cullingDispatcher->DeleteLater();
    m_cullingDispatcher = nullptr;
}

void MSceneCullingNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);
    MORTY_UNUSED(primaryCommand);

    auto instanceManager = info.scene->GetManager<MMeshInstanceManager>();
    instanceManager->GetBatchGroups();
}

std::vector<MRenderTaskOutputDesc> MSceneCullingNode::InitOutputDesc()
{
    static const auto outputCullingResultId = MStringId("Culling output");

    return {
            MRenderTaskNodeOutput::CreateBuffer(outputCullingResultId),
    };
}
