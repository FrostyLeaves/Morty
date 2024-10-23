#include "MDeepPeelRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
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

MORTY_CLASS_IMPLEMENT(MDeepPeelRenderNode, ISinglePassRenderNode)

const MStringId MDeepPeelRenderNode::FrontTextureOutput = MStringId("Deep Peel Front");
const MStringId MDeepPeelRenderNode::BackTextureOutput  = MStringId("Deep Peel Back");
const MStringId MDeepPeelRenderNode::DepthOutput[4]     = {
        MStringId("Deep Peel Front Depth Output 0"),
        MStringId("Deep Peel Back Depth Output 0"),
        MStringId("Deep Peel Front Buffer Output 1"),
        MStringId("Deep Peel Back Depth Output 1"),
};


void MDeepPeelRenderNode::OnCreated()
{
    Super::OnCreated();

    InitializeTexture();
    InitializeMaterial();
    InitializeFrameShaderParams();

    InitializeRenderPass();
}

void MDeepPeelRenderNode::Release()
{

    ReleaseFrameShaderParams();
    ReleaseMaterial();
    ReleaseTexture();

    Super::Release();
}

void MDeepPeelRenderNode::Render(const MRenderInfo& info)
{
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    //Render static mesh.
    MCullingResultRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({
            GetRenderGraph()->GetFrameProperty(),
    });
    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDepthPeel));
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

    Render(info, {&indirectMesh});
}

void MDeepPeelRenderNode::Render(const MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable)
{
    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
    if (!pCommand)
    {
        MORTY_ASSERT(pCommand);
        return;
    }

    MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
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
    pCommand->SetScissor(MScissorInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));


    if (pCommand->SetUseMaterial(m_drawPeelMaterial)) { pCommand->DrawMesh(pMeshManager->GetScreenRect()); }

    for (size_t nSubpassIdx = 1; nSubpassIdx < m_renderPass.m_subpass.size(); ++nSubpassIdx)
    {
        pCommand->NextSubPass();

        pCommand->PushShaderPropertyBlock(m_framePropertyBlock[nSubpassIdx % 2]);

        for (MCullingResultRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }

        pCommand->PopShaderPropertyBlock();
    }

    pCommand->EndRenderPass();
}

void MDeepPeelRenderNode::InitializeMaterial()
{
    MResourceSystem*           pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::shared_ptr<MResource> pDPVSResource  = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");
    std::shared_ptr<MResource> pDPFPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_fill.mps");

    const auto                 pPeelTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
    pPeelTemplate->SetMaterialType(MEMaterialType::EDepthPeel);
    pPeelTemplate->AddDefine(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
    pPeelTemplate->LoadShader(pDPVSResource);
    pPeelTemplate->LoadShader(pDPFPSResource);
    m_drawPeelMaterial = MMaterial::CreateMaterial(pPeelTemplate);
}

void MDeepPeelRenderNode::ReleaseMaterial() { m_drawPeelMaterial = nullptr; }

void MDeepPeelRenderNode::InitializeTexture()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    static MByte     black[4] = {0, 0, 0, 0};
    static MByte     white[4] = {255, 255, 255, 255};

    m_blackTexture = pResourceSystem->FindResource<MTextureResource>("Transparent_Black");
    if (m_blackTexture == nullptr)
    {
        m_blackTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_Black");
        m_blackTexture->Load(
                MTextureResourceUtil::LoadFromMemory("Transparent_Black", black, 1, 1, 4, MTexturePixelType::Byte8)
        );
    }

    m_whiteTexture = pResourceSystem->FindResource<MTextureResource>("Transparent_White");
    if (m_whiteTexture == nullptr)
    {
        m_whiteTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_White");
        m_whiteTexture->Load(
                MTextureResourceUtil::LoadFromMemory("Transparent_White", white, 1, 1, 4, MTexturePixelType::Byte8)
        );
    }
}

void MDeepPeelRenderNode::ReleaseTexture()
{
    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_blackTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());
    m_whiteTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());

    m_blackTexture = nullptr;
    m_whiteTexture = nullptr;
}

void MDeepPeelRenderNode::InitializeRenderPass()
{
#if MORTY_DEBUG
    m_renderPass.m_strDebugName = "Transparent Peel";
#endif

    static constexpr uint32_t SUB_PASS_NUM = 6;

    m_renderPass.m_subpass.push_back(MSubpass());
    MSubpass& subpass = m_renderPass.m_subpass.back();

    /*
    * 0 output front
    * 1 output back
    * 2 input/output front
    * 3 input/output back
    * 4 input/output front depth
    * 5 input/output back depth
    */

    subpass.m_inputIndex  = {};
    subpass.m_outputIndex = {0, 1, 2, 3};

    for (uint32_t i = 0; i < SUB_PASS_NUM; ++i)
    {
        m_renderPass.m_subpass.push_back(MSubpass());
        MSubpass& subpass = m_renderPass.m_subpass.back();

        if (i % 2)
        {
            subpass.m_inputIndex  = {4, 5};
            subpass.m_outputIndex = {0, 1, 2, 3};
        }
        else
        {
            subpass.m_inputIndex  = {2, 3};
            subpass.m_outputIndex = {0, 1, 4, 5};
        }
    }

    m_renderPass.SetDepthTestEnable(true);
    m_renderPass.SetDepthWriteEnable(false);
}

void MDeepPeelRenderNode::InitializeFrameShaderParams()
{
    MResourceSystem*           pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::shared_ptr<MResource> forwardVS = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
    std::shared_ptr<MResource> forwardPS = pResourceSystem->LoadResource("Shader/Forward/basic_lighting.mps");
    auto                       pTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();
    pTemplate->SetCullMode(MECullMode::ECullBack);
    pTemplate->SetMaterialType(MEMaterialType::EDepthPeel);
    pTemplate->AddDefine(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
    pTemplate->LoadShader(forwardVS);
    pTemplate->LoadShader(forwardPS);

    m_forwardMaterial = MMaterial::CreateMaterial(pTemplate);

    m_framePropertyBlock[0] = m_forwardMaterial->GetShaderProgram()
                                      ->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]
                                      ->Clone();
    m_framePropertyBlock[1] = m_forwardMaterial->GetShaderProgram()
                                      ->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]
                                      ->Clone();
}

void MDeepPeelRenderNode::ReleaseFrameShaderParams()
{
    const auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_framePropertyBlock[0]->DestroyBuffer(pRenderSystem->GetDevice());
    m_framePropertyBlock[1]->DestroyBuffer(pRenderSystem->GetDevice());

    m_forwardMaterial = nullptr;
}

void MDeepPeelRenderNode::BindInOutTexture()
{
    const auto pDepthTexture = GetInputTexture(0);
    m_drawPeelMaterial->GetMaterialPropertyBlock()->SetTexture(
            MShaderPropertyName::TRANSPARENT_TEXTURE_BACK_TEXTURE,
            pDepthTexture
    );

    m_framePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, GetOutputTexture(4));
    m_framePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, GetOutputTexture(5));
    m_framePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, GetOutputTexture(2));
    m_framePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, GetOutputTexture(3));

    Super::BindInOutTexture();
}

std::vector<MRenderTaskInputDesc> MDeepPeelRenderNode::InitInputDesc()
{
    return {
            {MForwardRenderNode::DepthBufferOutput, METextureFormat::Depth, METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MDeepPeelRenderNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::Create(MRenderTaskNode::DefaultLinearSpaceFormat, {true, MColor::Black_T}),
            MRenderTaskNodeOutput::Create(MRenderTaskNode::DefaultLinearSpaceFormat, {true, MColor::Black_T}),
            MRenderTaskNodeOutput::Create(METextureFormat::Float_R32, {true, MColor::White}),
            MRenderTaskNodeOutput::Create(METextureFormat::Float_R32, {true, MColor::Black_T}),
            MRenderTaskNodeOutput::Create(METextureFormat::Float_R32, {true, MColor::White}),
            MRenderTaskNodeOutput::Create(METextureFormat::Float_R32, {true, MColor::Black_T}),
    };
}
