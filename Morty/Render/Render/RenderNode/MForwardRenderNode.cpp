#include "MForwardRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MVRSTextureRenderNode.h"
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
#include "Manager/MEnvironmentManager.h"
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "Render/MeshRender/MCullingResultRenderable.h"
#include "Render/MeshRender/MSkyBoxRenderable.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Utility/MBounds.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MForwardRenderNode, ISinglePassRenderNode)

const MStringId MForwardRenderNode::BackBufferOutput  = MStringId("Forward Back");
const MStringId MForwardRenderNode::DepthBufferOutput = MStringId("Forward Depth");


void            MForwardRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);

    const Vector2i v2LeftTop = info.f2ViewportLeftTop;
    const Vector2i v2Size    = info.f2ViewportSize;
    pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }

    pCommand->EndRenderPass();
}

void MForwardRenderNode::Render(const MRenderInfo& info)
{

    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({
            GetRenderGraph()->GetFrameProperty(),
    });
    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDefault));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    const MEnvironmentManager* pEnvironmentManager = info.pScene->GetManager<MEnvironmentManager>();
    const auto                 pMaterial           = pEnvironmentManager->GetMaterial();

    MSkyBoxRenderable          skyBox;
    skyBox.SetMesh(pMeshManager->GetSkyBox());
    skyBox.SetMaterial(pMaterial);
    skyBox.SetPropertyBlockAdapter({GetRenderGraph()->GetFrameProperty()});

    Render(info,
           {
                   &indirectMesh,
                   &skyBox,
           });
}

void MForwardRenderNode::BindInOutTexture()
{
    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskInputDesc> MForwardRenderNode::InitInputDesc()
{
    return {
            {MDeferredLightingRenderNode::DeferredLightingOutput,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderWrite},
            {MGBufferRenderNode::GBufferDepthBufferOutput,
             METextureFormat::Depth,
             METextureBarrierStage::EPixelShaderWrite},
            {MShadowMapRenderNode::ShadowMapBufferOutput,
             METextureFormat::Depth,
             METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MForwardRenderNode::InitOutputDesc()
{
    return {MRenderTaskNodeOutput::CreateFromInput({false, MColor::Black_T}, 0),
            MRenderTaskNodeOutput::CreateFromInput({false, MColor::Black_T}, 1)};
}
