#include "MDeferredLightingRenderWork.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MHBAOBlurRenderWork.h"
#include "MHBAORenderWork.h"
#include "MVRSTextureRenderWork.h"
#include "MVoxelizerRenderWork.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MScene.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MBounds.h"
#include "Utility/MMaterialName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDeferredLightingRenderWork, ISinglePassRenderWork)

const MStringId MDeferredLightingRenderWork::DeferredLightingOutput = MStringId("Deferred Lighting Output");


void            MDeferredLightingRenderWork::Render(const MRenderInfo& info)
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

void MDeferredLightingRenderWork::Initialize(MEngine* pEngine)
{
    Super::Initialize(pEngine);

    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    const auto       pTemplate       = pResourceSystem->LoadResource(MMaterialName::DEFERRED_LIGHTING);
    m_lightningMaterial              = MMaterial::CreateMaterial(pTemplate);
}

void MDeferredLightingRenderWork::Release()
{
    m_lightningMaterial = nullptr;

    auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    if (auto pShadingRateTexture = m_renderPass.GetShadingRateTexture())
    {
        pShadingRateTexture->DestroyBuffer(pRenderSystem->GetDevice());
    }

    Super::Release();
}

void MDeferredLightingRenderWork::BindTarget()
{
    if (std::shared_ptr<MShaderPropertyBlock> pParams = m_lightningMaterial->GetMaterialPropertyBlock())
    {
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC,
                GetInputTexture(MGBufferRenderWork::GBufferAlbedoMetallic)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS,
                GetInputTexture(MGBufferRenderWork::GBufferNormalRoughness)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC,
                GetInputTexture(MGBufferRenderWork::GBufferPositionAmbientOcc)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_DEPTH_MAP,
                GetInputTexture(MGBufferRenderWork::GBufferDepthBufferOutput)
        );
        pParams->SetTexture(
                MShaderPropertyName::GBUFFER_TEXTURE_SSAO,
                GetInputTexture(MHBAOBlurRenderWorkH::BlurOutput)
        );
    }

    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskInputDesc> MDeferredLightingRenderWork::InitInputDesc()
{
    return
    {
        {MGBufferRenderWork::GBufferAlbedoMetallic, METextureBarrierStage::EPixelShaderSample},
                {MGBufferRenderWork::GBufferNormalRoughness, METextureBarrierStage::EPixelShaderSample},
                {MGBufferRenderWork::GBufferPositionAmbientOcc, METextureBarrierStage::EPixelShaderSample},
                {MShadowMapRenderWork::ShadowMapBufferOutput, METextureBarrierStage::EPixelShaderSample},
                {MHBAOBlurRenderWorkH::BlurOutput, METextureBarrierStage::EPixelShaderSample},
#if MORTY_VXGI_ENABLE
                {MVoxelizerRenderWork::VoxelizerBufferOutput, METextureBarrierStage::EUnknow},
#endif
    };
}

std::vector<MRenderTaskOutputDesc> MDeferredLightingRenderWork::InitOutputDesc()
{
    return {
            {DeferredLightingOutput, {true, MColor::Black_T}},
    };
}