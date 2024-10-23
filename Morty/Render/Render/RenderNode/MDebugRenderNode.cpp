#include "MDebugRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MToneMappingRenderNode.h"
#include "Material/MMaterial.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "Render/MeshRender/MCullingResultRenderable.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Utility/MBounds.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDebugRenderNode, ISinglePassRenderNode)

const MStringId MDebugRenderNode::BackBufferOutput  = MStringId("Debug Buffer");
const MStringId MDebugRenderNode::DepthBufferOutput = MStringId("Debug Depth");

void            MDebugRenderNode::Render(const MRenderInfo& info)
{
    //Current viewport.
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({GetRenderGraph()->GetFrameProperty()});
    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::ECustom));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    Render(info, {&indirectMesh});
}

void MDebugRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);

    const Vector2i v2LeftTop = info.f2ViewportLeftTop;
    const Vector2i v2Size    = info.f2ViewportSize;
    pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
    pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));


    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }


    pCommand->EndRenderPass();
}

std::vector<MRenderTaskInputDesc> MDebugRenderNode::InitInputDesc()
{
    return {{MToneMappingRenderNode::ToneMappingResult,
             METextureFormat::UNorm_RGBA8,
             METextureBarrierStage::EPixelShaderWrite},
            {MForwardRenderNode::DepthBufferOutput, METextureFormat::Depth, METextureBarrierStage::EPixelShaderWrite}};
}

std::vector<MRenderTaskOutputDesc> MDebugRenderNode::InitOutputDesc()
{
    return {MRenderTaskNodeOutput::CreateFromInput({false, MColor::Black_T}, 0),
            MRenderTaskNodeOutput::CreateFromInput({false, MColor::Black_T}, 1)};
}