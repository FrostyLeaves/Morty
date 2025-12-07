#include "MForwardRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Utility/MBounds.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MForwardRenderNode, ISinglePassRenderNode)


void MForwardRenderNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);
    MORTY_UNUSED(primaryCommand);

    auto command = primaryCommand->BeginRenderPass(&m_renderPass);

    command.SetViewportAndScissor({.rect = info.viewportRect});

    primaryCommand->EndRenderPass(command);
}

void MForwardRenderNode::BindInOutTexture()
{
    Super::AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskInputDesc> MForwardRenderNode::InitInputDesc()
{
    return {
            MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::ColorBuffer,
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    false
            ),                                                               // color buffer
            MRenderTaskNodeInput::CreateDepth(MRenderGraphName::DepthBuffer),//depth buffer
            MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::ShadowMap,
                    METextureFormat::Depth,
                    false
            ),//shadow map buffer
    };
}

std::vector<MRenderTaskOutputDesc> MForwardRenderNode::InitOutputDesc()
{
    return {MRenderTaskNodeOutput::CreateFromInput(MRenderGraphName::ColorBuffer, {false, MColor::Black_T}, 0),
            MRenderTaskNodeOutput::CreateFromInput(MRenderGraphName::DepthBuffer, {false, MColor::Black_T}, 1)};
}
