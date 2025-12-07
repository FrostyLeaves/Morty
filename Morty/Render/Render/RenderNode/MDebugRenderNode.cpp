#include "MDebugRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MToneMappingRenderNode.h"
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

MORTY_CLASS_IMPLEMENT(MDebugRenderNode, ISinglePassRenderNode)

void MDebugRenderNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);
    /*
    //Current viewport.
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetParameterSetAdapter({GetRenderGraph()->GetFrameProperty()});
    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::ECustom));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    Render(info, {&indirectMesh});

    */

    auto command = primaryCommand->BeginRenderPass(&m_renderPass);

    command.SetViewportAndScissor({.rect = info.viewportRect});

    primaryCommand->EndRenderPass(command);
}

std::vector<MRenderTaskInputDesc> MDebugRenderNode::InitInputDesc()
{
    return {
            MRenderTaskNodeInput::CreatePixelWrite(
                    MRenderGraphName::ColorBuffer,
                    METextureFormat::UNorm_RGBA8,
                    false
            ),                                                               // color buffer
            MRenderTaskNodeInput::CreateDepth(MRenderGraphName::DepthBuffer),// depth buffer
    };
}

std::vector<MRenderTaskOutputDesc> MDebugRenderNode::InitOutputDesc()
{
    return {MRenderTaskNodeOutput::CreateFromInput(MRenderGraphName::ColorBuffer, {false, MColor::Black_T}, 0),
            MRenderTaskNodeOutput::CreateFromInput(MRenderGraphName::DepthBuffer, {false, MColor::Black_T}, 1)};
}