#include "MShadowMapRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "Shadow/MShadowMapUtil.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MBounds.h"
#include "Utility/MRenderGraphName.h"


using namespace morty;

MORTY_CLASS_IMPLEMENT(MShadowMapRenderNode, ISinglePassRenderNode)

void MShadowMapRenderNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);
    MORTY_UNUSED(primaryCommand);
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
