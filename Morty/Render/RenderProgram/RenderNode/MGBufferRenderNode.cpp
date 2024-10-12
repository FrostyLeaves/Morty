#include "MGBufferRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Culling/MInstanceCulling.h"
#include "Engine/MEngine.h"
#include "MVRSTextureRenderNode.h"
#include "Mesh/MMeshManager.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "Scene/MScene.h"

#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MGBufferRenderNode, ISinglePassRenderNode)

const MStringId MGBufferRenderNode::GBufferAlbedoMetallic     = MStringId("GBuffer Albedo Metallic Buffer");
const MStringId MGBufferRenderNode::GBufferNormalRoughness    = MStringId("GBuffer Normal Roughness Buffer");
const MStringId MGBufferRenderNode::GBufferPositionAmbientOcc = MStringId("GBuffer Position AmbientOcc Buffer");
const MStringId MGBufferRenderNode::GBufferDepthBufferOutput  = MStringId("GBuffer Depth Buffer");

void            MGBufferRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    MIRenderCommand* pCommand  = info.pPrimaryRenderCommand;
    const Vector2i   v2LeftTop = info.f2ViewportLeftTop;
    const Vector2i   v2Size    = info.f2ViewportSize;

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);
    pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }


    pCommand->EndRenderPass();
}

class MORTY_API MGBufferTextures : public IGBufferAdapter
{
public:
    std::vector<std::shared_ptr<MTexture>> GetBackTextures() const { return vBackTextures; }
    std::shared_ptr<MTexture>              GetDepthTexture() const { return pDepthTexture; }

    std::vector<std::shared_ptr<MTexture>> vBackTextures;
    std::shared_ptr<MTexture>              pDepthTexture;
};

std::shared_ptr<IGBufferAdapter> MGBufferRenderNode::CreateGBuffer()
{
    auto pGBufferTextures           = std::make_shared<MGBufferTextures>();
    pGBufferTextures->vBackTextures = m_renderPass.GetBackTextures();
    pGBufferTextures->pDepthTexture = m_renderPass.GetDepthTexture();

    return pGBufferTextures;
}

void MGBufferRenderNode::Render(const MRenderInfo& info)
{
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
    //Camera frustum culling.

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({
            GetRenderGraph()->GetFrameProperty(),
    });

    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDeferred));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    Render(info,
           {
                   &indirectMesh,
           });
}

void MGBufferRenderNode::BindTarget()
{
    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskOutputDesc> MGBufferRenderNode::InitOutputDesc()
{
    return {
            {GBufferAlbedoMetallic, {true, MColor::Black_T}},
            {GBufferNormalRoughness, {true, MColor::Black_T}},
            {GBufferPositionAmbientOcc, {true, MColor::Black_T}},
            {GBufferDepthBufferOutput, {true, MColor::Black_T}},
    };
}
