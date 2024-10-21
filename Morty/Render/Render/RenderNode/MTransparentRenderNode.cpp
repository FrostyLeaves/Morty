#include "MTransparentRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MDeepPeelRenderNode.h"
#include "MForwardRenderNode.h"
#include "Model/MSkeleton.h"
#include "Model/MSkeletonInstance.h"
#include "RHI/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMaterialResource.h"
#include "Resource/MTextureResource.h"

#include "Component/MRenderMeshComponent.h"
#include "Culling/MInstanceCulling.h"

#include "Utility/MGlobal.h"
#include "Mesh/MMeshManager.h"
#include "Render/MeshRender/MCullingResultRenderable.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Resource/MTextureResourceUtil.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTransparentRenderNode, ISinglePassRenderNode)

const MStringId MTransparentRenderNode::BackBufferOutput = MStringId("Transparent Back");


void            MTransparentRenderNode::OnCreated()
{
    Super::OnCreated();

    InitializeMaterial();
    InitializeFillRenderPass();
}

void MTransparentRenderNode::Release()
{
    ReleaseMaterial();

    Super::Release();
}

void MTransparentRenderNode::Render(const MRenderInfo& info)
{
    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
    if (!pCommand)
    {
        MORTY_ASSERT(pCommand);
        return;
    }

    auto* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
    if (!pMeshManager)
    {
        MORTY_ASSERT(pMeshManager);
        return;
    }

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);

    const Vector2i f2LeftTop = info.f2ViewportLeftTop;
    const Vector2i f2Size    = info.f2ViewportSize;
    pCommand->SetViewport(MViewportInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, f2Size.x, f2Size.y));

    pCommand->SetUseMaterial(m_drawFillMaterial);

    pCommand->DrawMesh(pMeshManager->GetScreenRect());

    pCommand->EndRenderPass();
}

void MTransparentRenderNode::InitializeMaterial()
{
    MResourceSystem*           pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::shared_ptr<MResource> pDPVSResource  = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");
    std::shared_ptr<MResource> pDPBPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mps");


    const auto                 pFillTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
    pFillTemplate->SetMaterialType(MEMaterialType::ETransparentBlend);
    pFillTemplate->LoadShader(pDPVSResource);
    pFillTemplate->LoadShader(pDPBPSResource);
    m_drawFillMaterial = MMaterial::CreateMaterial(pFillTemplate);
}

void MTransparentRenderNode::ReleaseMaterial() { m_drawFillMaterial = nullptr; }

void MTransparentRenderNode::InitializeFillRenderPass()
{
#if MORTY_DEBUG
    m_renderPass.m_strDebugName = "Transparent Fill";
#endif

    m_renderPass.SetDepthTestEnable(false);
    m_renderPass.SetDepthWriteEnable(false);
}

void MTransparentRenderNode::BindTarget()
{
    std::vector<std::shared_ptr<MShaderTextureParam>>& params =
            m_drawFillMaterial->GetMaterialPropertyBlock()->m_textures;
    params[0]->SetTexture(GetInputTexture(MDeepPeelRenderNode::FrontTextureOutput));
    params[1]->SetTexture(GetInputTexture(MDeepPeelRenderNode::BackTextureOutput));

    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTarget());
}

std::vector<MRenderTaskInputDesc> MTransparentRenderNode::InitInputDesc()
{
    return {
            {MForwardRenderNode::BackBufferOutput,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderWrite},
            {MDeepPeelRenderNode::FrontTextureOutput,
             METextureFormat::UNorm_RGBA8,
             METextureBarrierStage::EPixelShaderSample},
            {MDeepPeelRenderNode::BackTextureOutput,
             METextureFormat::UNorm_RGBA8,
             METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MTransparentRenderNode::InitOutputDesc()
{
    return {{BackBufferOutput, MRenderTaskNode::DefaultLinearSpaceFormat, {false, MColor::Black_T}}};
}
