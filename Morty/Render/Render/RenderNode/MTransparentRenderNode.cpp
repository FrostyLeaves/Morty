#include "MTransparentRenderNode.h"
#include "Utility/MGlobal.h"
#include "Basic/MViewport.h"
#include "Component/MRenderMeshComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MRenderModule.h"
#include "Mesh/MMeshManager.h"
#include "Model/MSkeleton.h"
#include "Model/MSkeletonInstance.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "Render/MeshRender/MCullingResultRenderable.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialTemplateResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MTextureResourceUtil.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MMaterialName.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTransparentRenderNode, MRenderTaskNode)

const MStringId FrontTextureOutput = MStringId("Front");
const MStringId BackTextureOutput  = MStringId("Back");
const MStringId DepthOutput[4]     = {
        MStringId("Front Depth 0"),
        MStringId("Back Depth 0"),
        MStringId("Front Depth 1"),
        MStringId("Back Depth 1"),
};


void MTransparentRenderNode::OnCreated()
{
    Super::OnCreated();

    InitializeTexture();
    InitializeMaterial();
    InitializeRenderPass();
}

void MTransparentRenderNode::Release()
{
    ReleaseMaterial();
    ReleaseTexture();

    Super::Release();
}

void MTransparentRenderNode::Render(const MRenderInfo& info)
{
    DrawPeel(info);
    DrawFill(info);
}

void MTransparentRenderNode::InitializeMaterial()
{
    auto*                      pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    std::shared_ptr<MResource> pDPVSResource   = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");


    std::shared_ptr<MResource> pDPFPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_init.mps");
    const auto                 pPeelTemplate  = pResourceSystem->CreateResource<MMaterialTemplate>();
    pPeelTemplate->SetMaterialType(MEMaterialType::EDepthPeel);
    pPeelTemplate->AddDefine(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
    pPeelTemplate->LoadShader(pDPVSResource);
    pPeelTemplate->LoadShader(pDPFPSResource);
    m_copyDepthMaterial = MMaterial::CreateMaterial(pPeelTemplate);


    std::shared_ptr<MResource> pDPBPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mps");
    const auto                 pFillTemplate  = pResourceSystem->CreateResource<MMaterialTemplate>();
    pFillTemplate->SetMaterialType(MEMaterialType::ETransparentBlend);
    pFillTemplate->LoadShader(pDPVSResource);
    pFillTemplate->LoadShader(pDPBPSResource);
    m_blendMaterial = MMaterial::CreateMaterial(pFillTemplate);

    auto pTransparentTemplate =
            pResourceSystem->LoadResource(MMaterialName::FORWARD_TRANSPARENT)->DynamicCast<MMaterialTemplateResource>();
    m_framePropertyBlock[0] = pTransparentTemplate->GetShaderProgram()
                                      ->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]
                                      ->Clone();
    m_framePropertyBlock[1] = pTransparentTemplate->GetShaderProgram()
                                      ->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]
                                      ->Clone();
}

void MTransparentRenderNode::ReleaseMaterial()
{
    const auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_copyDepthMaterial = nullptr;
    m_blendMaterial     = nullptr;

    m_framePropertyBlock[0]->DestroyBuffer(pRenderSystem->GetDevice());
    m_framePropertyBlock[1]->DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderNode::InitializeRenderPass()
{

#if MORTY_DEBUG
    m_peelPass.m_strDebugName = "Transparent Peel";
    m_fillPass.m_strDebugName = "Transparent Fill";
#endif

    m_peelPass.SetDepthTestEnable(true);
    m_peelPass.SetDepthWriteEnable(false);
    m_fillPass.SetDepthTestEnable(false);
    m_fillPass.SetDepthWriteEnable(false);

    static constexpr uint32_t SUB_PASS_NUM = 6;

    m_peelPass.m_subpass.emplace_back();
    MSubpass& firstPass = m_peelPass.m_subpass.back();

    /*
    * 0 output front
    * 1 output back
    * 2 input/output front
    * 3 input/output back
    * 4 input/output front depth
    * 5 input/output back depth
    */

    firstPass.m_inputIndex  = {};
    firstPass.m_outputIndex = {0, 1, 2, 3};

    for (uint32_t i = 0; i < SUB_PASS_NUM; ++i)
    {
        m_peelPass.m_subpass.emplace_back();
        MSubpass& subPass = m_peelPass.m_subpass.back();

        if (i % 2)
        {
            subPass.m_inputIndex  = {4, 5};
            subPass.m_outputIndex = {0, 1, 2, 3};
        }
        else
        {
            subPass.m_inputIndex  = {2, 3};
            subPass.m_outputIndex = {0, 1, 4, 5};
        }
    }
}

void MTransparentRenderNode::InitializeTexture()
{
    auto* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    m_whiteTexture        = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
    m_blackTexture        = pResourceSystem->LoadResource(MRenderModule::DefaultBlack);
}
void MTransparentRenderNode::ReleaseTexture()
{
    m_blackTexture = nullptr;
    m_whiteTexture = nullptr;
}

void MTransparentRenderNode::DrawPeel(const MRenderInfo& info)
{
    IRenderCommand* pCommand = info.pPrimaryRenderCommand;
    if (!pCommand)
    {
        MORTY_ASSERT(pCommand);
        return;
    }

    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({
            GetRenderGraph()->GetFrameProperty(),
    });
    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDepthPeel));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    auto          command = pCommand->BeginRenderPass(&m_peelPass);

    const Vector2 f2LeftTop = info.f2ViewportLeftTop;
    const Vector2 f2Size    = info.f2ViewportSize;

    command.SetViewportAndScissor({.x = f2LeftTop.x, .y = f2LeftTop.y, .width = f2Size.x, .height = f2Size.y});

    command.SetMaterial(m_copyDepthMaterial.get());
    command.DrawMesh(pMeshManager->GetScreenRect());

    for (size_t nSubpassIdx = 1; nSubpassIdx < m_peelPass.m_subpass.size(); ++nSubpassIdx)
    {
        command.NextSubPass();

        command.PushShaderPropertyBlock(m_framePropertyBlock[nSubpassIdx % 2].get());
        indirectMesh.Render(&command);

        command.PopShaderPropertyBlock();
    }

    pCommand->EndRenderPass(command);
}

void MTransparentRenderNode::DrawFill(const MRenderInfo& info)
{
    IRenderCommand* pCommand = info.pPrimaryRenderCommand;
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

    auto          command = pCommand->BeginRenderPass(&m_fillPass);

    const Vector2 f2LeftTop = info.f2ViewportLeftTop;
    const Vector2 f2Size    = info.f2ViewportSize;
    command.SetViewport({.x = f2LeftTop.x, .y = f2LeftTop.y, .width = f2Size.x, .height = f2Size.y});
    command.SetScissor({.x = 0.0f, .y = 0.0f, .width = f2Size.x, .height = f2Size.y});

    command.SetMaterial(m_blendMaterial.get());

    command.DrawMesh(pMeshManager->GetScreenRect());

    pCommand->EndRenderPass(command);
}

void MTransparentRenderNode::BindInOutTexture()
{
    Super::AutoBindBarrierTexture();

    const auto pDepthTexture = GetInputTexture(1);// depth buffer
    m_copyDepthMaterial->GetMaterialPropertyBlock()->SetTexture(
            MShaderPropertyName::TRANSPARENT_TEXTURE_BACK_TEXTURE,
            pDepthTexture
    );

    if (auto texture = GetOutputTexture(5))
    {
        m_framePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, texture);
    }
    if (auto texture = GetOutputTexture(6))
    {
        m_framePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, texture);
    }
    if (auto texture = GetOutputTexture(3))
    {
        m_framePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, texture);
    }
    if (auto texture = GetOutputTexture(4))
    {
        m_framePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, texture);
    }

    auto& params = m_blendMaterial->GetMaterialPropertyBlock()->m_textures;

    if (auto texture = GetOutputTexture(1)) { params[0]->SetTexture(texture); }
    if (auto texture = GetOutputTexture(2)) { params[1]->SetTexture(texture); }

    m_peelPass.SetRenderTarget(
            {.backTargets =
                     {
                             GetRenderOutput(1)->CreateRenderTarget(),
                             GetRenderOutput(2)->CreateRenderTarget(),
                             GetRenderOutput(3)->CreateRenderTarget(),
                             GetRenderOutput(4)->CreateRenderTarget(),
                             GetRenderOutput(5)->CreateRenderTarget(),
                             GetRenderOutput(6)->CreateRenderTarget(),
                     },
             .depthTarget = {},
             .shadingRate = {}}
    );

    m_fillPass.SetRenderTarget(
            {.backTargets = {GetRenderOutput(0)->CreateRenderTarget()}, .depthTarget = {}, .shadingRate = {}}
    );


    auto* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_peelPass.DestroyBuffer(pRenderSystem->GetDevice());
    m_peelPass.GenerateBuffer(pRenderSystem->GetDevice());
    m_fillPass.DestroyBuffer(pRenderSystem->GetDevice());
    m_fillPass.GenerateBuffer(pRenderSystem->GetDevice());
}

std::vector<MRenderTaskInputDesc> MTransparentRenderNode::InitInputDesc()
{
    return {
            MRenderTaskNodeInput::CreatePixelWrite(
                    MRenderGraphName::ColorBuffer,
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    false
            ),// color buffer
            MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::DepthBuffer,
                    METextureFormat::Depth,
                    false
            ),// depth buffer
    };
}

std::vector<MRenderTaskOutputDesc> MTransparentRenderNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::CreateFromInput(
                    MRenderGraphName::ColorBuffer,
                    {false, MColor::Black_T},
                    0
            ),// Color Buffer
            MRenderTaskNodeOutput::Create(
                    FrontTextureOutput,
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    {true, MColor::Black_T}
            ),// Front Buffer
            MRenderTaskNodeOutput::Create(
                    BackTextureOutput,
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    {true, MColor::Black_T}
            ),// Back Buffer
            MRenderTaskNodeOutput::Create(DepthOutput[0], METextureFormat::Float_R32, {true, MColor::White}),
            MRenderTaskNodeOutput::Create(DepthOutput[1], METextureFormat::Float_R32, {true, MColor::Black_T}),
            MRenderTaskNodeOutput::Create(DepthOutput[2], METextureFormat::Float_R32, {true, MColor::White}),
            MRenderTaskNodeOutput::Create(DepthOutput[3], METextureFormat::Float_R32, {true, MColor::Black_T}),
    };
}
void MTransparentRenderNode::Resize(Vector2i size)
{
    Super::Resize(size);

    auto* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    if (m_peelPass.GetFrameBufferSize() != size) { m_peelPass.Resize(pRenderSystem->GetDevice()); }
    if (m_fillPass.GetFrameBufferSize() != size) { m_fillPass.Resize(pRenderSystem->GetDevice()); }
}
