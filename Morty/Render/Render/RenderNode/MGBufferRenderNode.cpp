#include "MGBufferRenderNode.h"
#include "MFrameParameterSetAdapter.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"
#include "Render/MRenderer.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Shader/MShaderParameterSet.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MRenderGraphName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MGBufferRenderNode, ISinglePassRenderNode)

class MORTY_API MGBufferTextures : public IGBufferAdapter
{
public:
    [[nodiscard]] MTextureArray GetBackTextures() const override { return vBackTextures; }
    [[nodiscard]] MTexturePtr   GetDepthTexture() const override { return pDepthTexture; }

    MTextureArray               vBackTextures;
    MTexturePtr                 pDepthTexture;
};

std::shared_ptr<IGBufferAdapter> MGBufferRenderNode::CreateGBuffer()
{
    auto pGBufferTextures           = std::make_shared<MGBufferTextures>();
    pGBufferTextures->vBackTextures = m_renderPass.GetBackTextures();
    pGBufferTextures->pDepthTexture = m_renderPass.GetDepthTexture();

    return pGBufferTextures;
}

void MGBufferRenderNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);
    MORTY_UNUSED(primaryCommand);


    auto command = primaryCommand->BeginRenderPass(&m_renderPass);
    command.SetViewportAndScissor({.rect = info.viewportRect});

    // Push frame parameter set if available
    if (auto adapter = GetRenderInput(FrameParamSetInputIdx)->GetData<MFrameParameterSetAdapter>())
    {
        if (auto paramSet = adapter->GetParameterSet())
        {
            command.PushShaderParameterSet(paramSet.get());
        }
    }

    if (auto renderer = GetRenderInput(RendererInputIdx)->GetData<IRenderer>())
    {
        renderer->Execute(&command);
    }

    // Pop frame parameter set if it was pushed
    if (auto adapter = GetRenderInput(FrameParamSetInputIdx)->GetData<MFrameParameterSetAdapter>())
    {
        if (adapter->GetParameterSet())
        {
            command.PopShaderParameterSet();
        }
    }

    primaryCommand->EndRenderPass(command);
}

void MGBufferRenderNode::BindInOutTexture()
{
    Super::AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskInputDesc> MGBufferRenderNode::InitInputDesc()
{
    return {
            MRenderTaskNodeInput::CreateData<IRenderer>(MRenderGraphName::Renderer),
            MRenderTaskNodeInput::CreateData<MFrameParameterSetAdapter>(MRenderGraphName::FrameParamSet),
    };
}

std::vector<MRenderTaskOutputDesc> MGBufferRenderNode::InitOutputDesc()
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
