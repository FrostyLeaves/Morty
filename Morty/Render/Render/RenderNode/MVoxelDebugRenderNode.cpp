#include "MVoxelDebugRenderNode.h"

#include "Utility/MRenderGlobal.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MVoxelizerRenderNode.h"
#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MScene.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "Render/MFrameShaderPropertyBlock.h"
#include "Render/MeshRender/MIndirectIndexRenderable.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Utility/MBounds.h"
#include "VXGI/MVoxelMapUtil.h"
#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MVoxelDebugRenderNode, ISinglePassRenderNode)


const MStringId MVoxelDebugRenderNode::BackBufferOutput  = MStringId("Voxel Debug Back");
const MStringId MVoxelDebugRenderNode::DepthBufferOutput = MStringId("Voxel Debug Depth");


void            MVoxelDebugRenderNode::Initialize(MEngine* pEngine)
{
    Super::Initialize(pEngine);

    InitializeBuffer();
    InitializeDispatcher();


    m_renderPass.SetDepthTestEnable(true);
    m_renderPass.SetDepthWriteEnable(true);
}

void MVoxelDebugRenderNode::Release()
{
    ReleaseDispatcher();
    ReleaseBuffer();

    Super::Release();
}

std::shared_ptr<IShaderPropertyUpdateDecorator> MVoxelDebugRenderNode::GetFramePropertyDecorator()
{
    return m_framePropertyUpdateDecorator;
}

const MBuffer* MVoxelDebugRenderNode::GetVoxelDebugBuffer() const { return &m_drawIndirectBuffer; }

void           MVoxelDebugRenderNode::Render(const MRenderInfo& info)
{
    const MMeshManager*      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    auto                     pVoxelizerWork = GetInput(0)->GetLinkedNode()->DynamicCast<MVoxelizerRenderNode>();

    MIndirectIndexRenderable debugRender;
    debugRender.SetMaterial(GetVoxelDebugMaterial());
    debugRender.SetPropertyBlockAdapter({GetRenderGraph()->GetFrameProperty()});
    debugRender.SetIndirectIndexBuffer(GetVoxelDebugBuffer());
    debugRender.SetMeshBuffer(pMeshManager->GetMeshBuffer());

    Render(info,
           pVoxelizerWork->GetVoxelSetting(),
           pVoxelizerWork->GetVoxelTableBuffer(),
           {
                   &debugRender,
           });
}

void MVoxelDebugRenderNode::Render(
        const MRenderInfo&               info,
        const MVoxelMapSetting&          voxelSetting,
        const MBuffer*                   pVoxelizerBuffer,
        const std::vector<IRenderable*>& vRenderable
)
{
    const int nDebugClipmapIdx = 2;

    if (nDebugClipmapIdx != voxelSetting.nClipmapIdx) { return; }

    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

    if (m_debugVoxelMapSetting)
    {
        auto& settingStruct = m_debugVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>(
                MShaderPropertyName::VOXEL_MAP_SETTING
        );
        MRenderInfo::FillVoxelMapSetting(voxelSetting, settingStruct);

        m_debugVoxelMapSetting->SetDirty();
    }

    if (m_voxelStorageBuffer) { m_voxelStorageBuffer->SetBuffer(pVoxelizerBuffer); }

    pCommand->AddBufferMemoryBarrier(
            {&m_drawIndirectBuffer},
            MEBufferBarrierStage::EDrawIndirectRead,
            MEBufferBarrierStage::EComputeShaderWrite
    );

    pCommand->DispatchComputeJob(
            m_voxelDebugIndirectGenerator,
            MRenderGlobal::VOXEL_TABLE_SIZE / 8,
            MRenderGlobal::VOXEL_TABLE_SIZE / 8,
            MRenderGlobal::VOXEL_TABLE_SIZE / 8
    );

    pCommand->AddBufferMemoryBarrier(
            {&m_drawIndirectBuffer},
            MEBufferBarrierStage::EComputeShaderWrite,
            MEBufferBarrierStage::EDrawIndirectRead
    );

    pCommand->AddBufferMemoryBarrier(
            {pVoxelizerBuffer},
            MEBufferBarrierStage::EComputeShaderRead,
            MEBufferBarrierStage::EPixelShaderWrite
    );

    const Vector2i v2LeftTop = info.f2ViewportLeftTop;
    const Vector2i v2Size    = info.f2ViewportSize;

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);
    pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }

    pCommand->EndRenderPass();
}

void MVoxelDebugRenderNode::InitializeBuffer()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    const int      nVoxelCubeMapSize =
            MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE;

    const size_t nDrawIndirectBufferSize = sizeof(MDrawIndexedIndirectData) * nVoxelCubeMapSize;
    m_drawIndirectBuffer                 = MBuffer::CreateIndirectDrawBuffer("Voxel Draw Indirect Buffer");
    m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
    m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);
}

void MVoxelDebugRenderNode::ReleaseBuffer()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MVoxelDebugRenderNode::InitializeDispatcher()
{
    MObjectSystem*   pObjectSystem   = GetEngine()->FindSystem<MObjectSystem>();
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();


    m_voxelDebugIndirectGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
    m_voxelDebugIndirectGenerator->LoadComputeShader("Shader/Voxel/voxel_debug_view.mcs");

    const std::shared_ptr<MShaderPropertyBlock>& params = m_voxelDebugIndirectGenerator->GetShaderPropertyBlock(0);
    MORTY_ASSERT(
            m_debugVoxelMapSetting = params->FindConstantParam(MShaderPropertyName::VOXELIZER_CBUFFER_VOXEL_MAP_NAME)
    );
    MORTY_ASSERT(m_voxelStorageBuffer = params->FindStorageParam(MShaderPropertyName::VOXELIZER_VOXEL_TABLE_NAME));

    const auto pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
    const auto cubeMesh     = pMeshManager->GetCubeMesh();
    params->SetValue(
            MShaderPropertyName::VOXELIZER_CUBE_MESH_INDEX,
            static_cast<uint32_t>(cubeMesh.indexMemoryInfo.begin / sizeof(uint32_t))
    );
    params->SetValue(
            MShaderPropertyName::VOXELIZER_CUBE_MESH_COUNT,
            static_cast<uint32_t>(cubeMesh.indexMemoryInfo.size / sizeof(uint32_t))
    );

    if (auto pIndirectDraws = params->FindStorageParam(MShaderPropertyName::CULLING_OUTPUT_DRAW_DATA))
    {
        pIndirectDraws->SetBuffer(&m_drawIndirectBuffer);
    }

    std::shared_ptr<MResource> voxelDebugVS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mvs");
    std::shared_ptr<MResource> voxelDebugPS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mps");
    auto pVoxelDebugTemplate = pResourceSystem->CreateResource<MMaterialTemplate>("Voxel Debug Material");
    pVoxelDebugTemplate->SetCullMode(MECullMode::ECullBack);
    pVoxelDebugTemplate->SetMaterialType(MEMaterialType::ECustom);
    pVoxelDebugTemplate->LoadShader(voxelDebugVS);
    pVoxelDebugTemplate->LoadShader(voxelDebugPS);

    m_voxelDebugMaterial = MMaterial::CreateMaterial(pVoxelDebugTemplate);
}

void MVoxelDebugRenderNode::ReleaseDispatcher()
{
    m_voxelDebugIndirectGenerator->DeleteLater();
    m_voxelDebugIndirectGenerator = nullptr;

    m_voxelDebugMaterial = nullptr;
}

void MVoxelDebugRenderNode::BindTarget()
{
    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTarget());
}

std::vector<MRenderTaskInputDesc> MVoxelDebugRenderNode::InitInputDesc()
{
    return {
            {MVoxelizerRenderNode::VoxelizerBufferOutput, METextureBarrierStage::EUnknow},
    };
}

std::vector<MRenderTaskOutputDesc> MVoxelDebugRenderNode::InitOutputDesc()
{
    return {{BackBufferOutput, {true, MColor::Black_T}}, {DepthBufferOutput, {true, MColor::Black_T}}};
}
