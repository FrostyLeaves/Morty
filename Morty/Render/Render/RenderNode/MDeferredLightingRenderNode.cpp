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
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
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

void MDeferredLightingRenderNode::Render(const MRenderInfo& info)
{
    UpdateProperty();

    auto* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
    if (!pMeshManager)
    {
        MORTY_ASSERT(pMeshManager);
        return;
    }

    IRenderCommand* pCommand = info.pPrimaryRenderCommand;
    auto            command  = pCommand->BeginRenderPass(&m_renderPass);

    //pCommand->SetShadingRate({ 1, 1 }, { MEShadingRateCombinerOp::Keep, MEShadingRateCombinerOp::Replace });

    const Vector2i  n2Size = m_renderPass.GetFrameBufferSize();

    command.SetViewportAndScissor(
            {.x = 0.0f, .y = 0.0f, .width = static_cast<float>(n2Size.x), .height = static_cast<float>(n2Size.y)}
    );

    command.SetMaterial(m_lightningMaterial.get());
    auto pPropertyBlock = GetRenderGraph()->GetFrameProperty()->GetPropertyBlock();
    command.SetShaderPropertyBlock(pPropertyBlock);

    command.DrawMesh(pMeshManager->GetScreenRect());

    pCommand->EndRenderPass(command);
}

void MDeferredLightingRenderNode::OnCreated()
{
    Super::OnCreated();

    auto       pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    const auto pTemplate       = pResourceSystem->LoadResource(MMaterialName::DEFERRED_LIGHTING);
    m_lightningMaterial        = MMaterial::CreateMaterial(pTemplate);
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

void MDeferredLightingRenderNode::UpdateProperty()
{
    if (LightingMaterial != nullptr && m_lightningMaterial.get() != LightingMaterial->DynamicCast<MMaterial>())
    {
        m_lightningMaterial =
                MMaterial::CreateMaterial(LightingMaterial->DynamicCast<MMaterial>()->GetMaterialTemplate());
        
        BindInOutTexture();
    }
}

void MDeferredLightingRenderNode::BindInOutTexture()
{
    Super::AutoBindBarrierTexture();

    if (std::shared_ptr<MShaderPropertyBlock> pParams = m_lightningMaterial->GetMaterialPropertyBlock())
    {
        if (auto texture = GetInputTexture(0))
        {
            pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC, texture);
        }
        if (auto texture = GetInputTexture(1))
        {
            pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS, texture);
        }
        if (auto texture = GetInputTexture(2))
        {
            pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC, texture);
        }
        if (auto texture = GetInputTexture(3))
        {
            pParams->SetTexture(MShaderPropertyName::TEXTURE_SHADOW_MAP, texture);
        }
        if (auto texture = GetInputTexture(4))
        {
            pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_SSAO, texture);
        }
    }

    SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskInputDesc> MDeferredLightingRenderNode::InitInputDesc()
{
    std::vector<MRenderTaskInputDesc> result = {
            MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::GBuffer[0],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    false
            ),
            MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::GBuffer[1],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    false
            ),
            MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::GBuffer[2],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    false
            ),
            MRenderTaskNodeInput::CreateSample(MRenderGraphName::ShadowMap, METextureFormat::Depth, true),
            MRenderTaskNodeInput::CreateSample(MRenderGraphName::TextureAO, METextureFormat::Depth, true),
    };
#if MORTY_VXGI_ENABLE
    {MVoxelizerRenderNode::VoxelizerBufferOutput, METextureBarrierStage::EUnknow},
#endif

            return result;
}

std::vector<MRenderTaskOutputDesc> MDeferredLightingRenderNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::ColorBuffer,
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    {true, MColor::Black_T}
            ),
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

void MDeferredLightingRenderNode::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto fbNode = fbs::GetMDeferredLightingRenderNode(fbb.GetCurrentBufferPointer());
    Deserialize(fbNode);
}

void MDeferredLightingRenderNode::Deserialize(const void* flatbuffer)
{
    const auto* fbDeferredNode = reinterpret_cast<const fbs::MDeferredLightingRenderNode*>(flatbuffer);
    MRenderTaskNode::Deserialize(fbDeferredNode->super());

    EnableAO = fbDeferredNode->enable_ao();
}
