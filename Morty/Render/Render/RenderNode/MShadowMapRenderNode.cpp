#include "MShadowMapRenderNode.h"

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
#include "Mesh/MVertex.h"
#include "Mesh/MMeshManager.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Utility/MBounds.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "Shadow/MShadowMapUtil.h"
#include "Utility/MRenderGraphName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShadowMapRenderNode, ISinglePassRenderNode)

void MShadowMapRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    IRenderCommand* pCommand = info.pPrimaryRenderCommand;
    if (!pCommand) return;

    const auto&    shadowMap = m_renderPass.GetDepthTexture();
    auto           command   = pCommand->BeginRenderPass(&m_renderPass);
    const Vector2i v2Size    = shadowMap->GetSize2D();

    command.SetViewportAndScissor(
            {.x = 0.0f, .y = 0.0f, .width = static_cast<float>(v2Size.x), .height = static_cast<float>(v2Size.y)}
    );

    MORTY_UNUSED(vRenderable);

    pCommand->EndRenderPass(command);
}

void MShadowMapRenderNode::Render(const MRenderInfo& info)
{
        MORTY_UNUSED(info);
        //TODO
}

void MShadowMapRenderNode::OnCreated()
{
    Super::OnCreated();
    m_renderPass.SetViewportNum(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
}

std::vector<MRenderTaskOutputDesc> MShadowMapRenderNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::ShadowMap,
                    MTexture::CreateShadowMapArray(
                            "Cascaded Shadow Map",
                            MRenderGlobal::SHADOW_TEXTURE_SIZE,
                            MRenderGlobal::CASCADED_SHADOW_MAP_NUM
                    ),
                    {true, MColor::White}
            ),
    };
}
