#include "MShadowMapRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
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
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Utility/MBounds.h"

#include "Utility/MGlobal.h"
#include "Shadow/MShadowMapUtil.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShadowMapRenderNode, ISinglePassRenderNode)
const MStringId MShadowMapRenderNode::ShadowMapBufferOutput = MStringId("Shadow Map Buffer Output");

class ShadowMapTexture : public IGetTextureAdapter
{
public:
    virtual std::shared_ptr<MTexture> GetTexture() { return pTexture; }

    std::shared_ptr<MTexture>         pTexture;
};

void MShadowMapRenderNode::Initialize(MEngine* pEngine)
{
    Super::Initialize(pEngine);
    m_renderPass.SetViewportNum(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
}

void MShadowMapRenderNode::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
    if (!pCommand) return;

    AutoSetTextureBarrier(pCommand);

    const auto& pShadowmap = m_renderPass.GetDepthTexture();

    pCommand->BeginRenderPass(&m_renderPass);

    const Vector2i v2LeftTop = Vector2i(0, 0);
    const Vector2i v2Size    = pShadowmap->GetSize2D();
    pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
    pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }

    pCommand->EndRenderPass();

    //pCommand->AddRenderToTextureBarrier({ m_renderPass.GetDepthTexture().get() }, METextureBarrierStage::EPixelShaderSample);
}

void MShadowMapRenderNode::Render(const MRenderInfo& info)
{
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({
            GetRenderGraph()->GetFrameProperty(),
    });
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetShadowCullingResult());

    Render(info,
           {
                   &indirectMesh,
           });
}

std::shared_ptr<IGetTextureAdapter> MShadowMapRenderNode::GetShadowMap() const
{
    auto pShadowMap      = std::make_shared<ShadowMapTexture>();
    pShadowMap->pTexture = m_renderPass.GetDepthTexture();

    return pShadowMap;
}

class MORTY_API MShadowPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
    explicit MShadowPropertyDecorator(MShadowMapRenderNode* pOwner)
        : m_owner(pOwner)
    {}

    void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override
    {
        MORTY_ASSERT(
                m_shadowTextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_SHADOW_MAP)
        );
    }

    void Update(const MRenderInfo& info) override
    {
        MORTY_UNUSED(info);

        const auto pTexture = m_owner->GetShadowMap()->GetTexture();
        if (m_shadowTextureParam && m_shadowTextureParam->GetTexture() != pTexture)
        {
            m_shadowTextureParam->SetTexture(pTexture);
        }
    }

    std::shared_ptr<MShaderTextureParam> m_shadowTextureParam = nullptr;

    MShadowMapRenderNode*                m_owner = nullptr;
};

std::shared_ptr<IShaderPropertyUpdateDecorator> MShadowMapRenderNode::GetFramePropertyDecorator()
{
    return std::make_shared<MShadowPropertyDecorator>(this);
}

void MShadowMapRenderNode::OnCreated() { Super::OnCreated(); }

void MShadowMapRenderNode::BindTarget()
{
    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTarget());
}

std::vector<MRenderTaskOutputDesc> MShadowMapRenderNode::InitOutputDesc()
{
    return {
            {ShadowMapBufferOutput, {true, MColor::White}},
    };
}
