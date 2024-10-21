#include "MDeferredLightingRenderNode.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MHBAOBlurRenderNode.h"
#include "MHBAORenderNode.h"
#include "MVRSTextureRenderNode.h"
#include "MVoxelizerRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MScene.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MBounds.h"
#include "Utility/MMaterialName.h"
#include "Flatbuffer/MDeferredLightingRenderNode_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDeferredLightingRenderNode, ISinglePassRenderNode)

const MStringId MDeferredLightingRenderNode::DeferredLightingOutput = MStringId("Deferred Lighting");


void            MDeferredLightingRenderNode::Render(const MRenderInfo& info)
{
    MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
    if (!pMeshManager)
    {
        MORTY_ASSERT(pMeshManager);
        return;
    }

    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);

    //pCommand->SetShadingRate({ 1, 1 }, { MEShadingRateCombinerOp::Keep, MEShadingRateCombinerOp::Replace });

    const Vector2i n2Size = m_renderPass.GetFrameBufferSize();

    pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


    if (pCommand->SetUseMaterial(m_lightningMaterial))
    {
        auto pPropertyBlock = GetRenderGraph()->GetFrameProperty()->GetPropertyBlock();
        pCommand->SetShaderPropertyBlock(pPropertyBlock);

        pCommand->DrawMesh(pMeshManager->GetScreenRect());
    }

    pCommand->EndRenderPass();
}

void MDeferredLightingRenderNode::OnCreated()
{
    Super::OnCreated();

    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    const auto       pTemplate       = pResourceSystem->LoadResource(MMaterialName::DEFERRED_LIGHTING);
    m_lightningMaterial              = MMaterial::CreateMaterial(pTemplate);
}

void MDeferredLightingRenderNode::Release()
{
    m_lightningMaterial = nullptr;

    auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    if (auto pShadingRateTexture = m_renderPass.GetShadingRateTexture())
    {
        pShadingRateTexture->DestroyBuffer(pRenderSystem->GetDevice());
    }

    Super::Release();
}

void MDeferredLightingRenderNode::BindTarget()
{
    if (std::shared_ptr<MShaderPropertyBlock> pParams = m_lightningMaterial->GetMaterialPropertyBlock())
    {
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC,
                GetInputTexture(MGBufferRenderNode::GBufferAlbedoMetallic)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS,
                GetInputTexture(MGBufferRenderNode::GBufferNormalRoughness)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC,
                GetInputTexture(MGBufferRenderNode::GBufferPositionAmbientOcc)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_DEPTH_MAP,
                GetInputTexture(MGBufferRenderNode::GBufferDepthBufferOutput)
        );

        if (EnableAO)
        {
            pParams->SetTexture(
                    MShaderPropertyName::GBUFFER_TEXTURE_SSAO,
                    GetInputTexture(MHBAOBlurRenderNodeH::BlurOutput)
            );
        }
    }

    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskInputDesc> MDeferredLightingRenderNode::InitInputDesc()
{
    std::vector<MRenderTaskInputDesc> result = {
            {MGBufferRenderNode::GBufferAlbedoMetallic,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderSample},
            {MGBufferRenderNode::GBufferNormalRoughness,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderSample},
            {MGBufferRenderNode::GBufferPositionAmbientOcc,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderSample},
            {MShadowMapRenderNode::ShadowMapBufferOutput,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderSample}
    };
    if (EnableAO)
    {
        result.push_back(
                {MHBAOBlurRenderNodeH::BlurOutput, METextureFormat::Depth, METextureBarrierStage::EPixelShaderSample}
        );
    }
#if MORTY_VXGI_ENABLE
    {MVoxelizerRenderNode::VoxelizerBufferOutput, METextureBarrierStage::EUnknow},
#endif

            return result;
}

std::vector<MRenderTaskOutputDesc> MDeferredLightingRenderNode::InitOutputDesc()
{
    return {
            {DeferredLightingOutput, MRenderTaskNode::DefaultLinearSpaceFormat, {true, MColor::Black_T}},
    };
}
flatbuffers::Offset<void> MDeferredLightingRenderNode::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                                    super = MRenderTaskNode::Serialize(fbb);

    fbs::MDeferredLightingRenderNodeBuilder builder(fbb);
    builder.add_super(super.o);
    builder.add_enable_ao(EnableAO);

    return builder.Finish().Union();
}
void MDeferredLightingRenderNode::Deserialize(const void* flatbuffer) { MRenderTaskNode::Deserialize(flatbuffer); }