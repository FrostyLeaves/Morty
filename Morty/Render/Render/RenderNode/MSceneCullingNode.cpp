#include "MSceneCullingNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"

#include "RHI/Command/MRenderPassCmd.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSceneCullingNode, ISinglePassRenderNode)

void MSceneCullingNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);
    MORTY_UNUSED(primaryCommand);
    //Camera frustum culling.

    //TODO
}

std::vector<MRenderTaskOutputDesc> MSceneCullingNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::GBuffer[0],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    {true, MColor::Black_T}
            ),
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::GBuffer[1],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    {true, MColor::Black_T}
            ),
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::GBuffer[2],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    {true, MColor::Black_T}
            ),
            MRenderTaskNodeOutput::CreateDepth(MRenderGraphName::DepthBuffer, {true, MColor::Black_T}),
    };
}
