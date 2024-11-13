#include "MGBufferRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Culling/MInstanceCulling.h"
#include "Engine/MEngine.h"
#include "MVRSTextureRenderNode.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/MeshRender/MCullingResultRenderable.h"
#include "Scene/MScene.h"

#include "RHI/Command/MRenderPassCmd.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MGBufferRenderNode, ISinglePassRenderNode)


void MGBufferRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    IRenderCommand* pCommand  = info.pPrimaryRenderCommand;
    const Vector2   v2LeftTop = info.f2ViewportLeftTop;
    const Vector2   v2Size    = info.f2ViewportSize;

    auto            command = pCommand->BeginRenderPass(&m_renderPass);
    command.SetViewport({.x = v2LeftTop.x, .y = v2LeftTop.y, .width = v2Size.x, .height = v2Size.y});
    command.SetScissor({.x = 0.0f, .y = 0.0f, .width = v2Size.x, .height = v2Size.y});

    for (IRenderable* renderable: vRenderable) { renderable->Render(&command); }

    pCommand->EndRenderPass(command);
}

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

void MGBufferRenderNode::BindInOutTexture()
{
    Super::AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
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
