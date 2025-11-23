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

void MDebugRenderNode::Render(const MRenderInfo& info)
{
    MORTY_UNUSED(info);
    /*
    //Current viewport.
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({GetRenderGraph()->GetFrameProperty()});
    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::ECustom));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    Render(info, {&indirectMesh});

    */
}

void MDebugRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    IRenderCommand* pCommand = info.pPrimaryRenderCommand;
    auto            command  = pCommand->BeginRenderPass(&m_renderPass);

    const Vector2   v2LeftTop = info.f2ViewportLeftTop;
    const Vector2   v2Size    = info.f2ViewportSize;
    command.SetViewportAndScissor({.x = v2LeftTop.x, .y = v2LeftTop.y, .width = v2Size.x, .height = v2Size.y});

    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(&command); }

    pCommand->EndRenderPass(command);
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