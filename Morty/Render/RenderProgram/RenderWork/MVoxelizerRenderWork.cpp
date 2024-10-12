#include "MVoxelizerRenderWork.h"

#include "Utility/MRenderGlobal.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MShadowMapRenderWork.h"
#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MScene.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"
#include "RenderProgram/MeshRender/MCullingResultSpecificMaterialRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Utility/MBounds.h"
#include "VXGI/MVoxelMapUtil.h"
#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MVoxelizerRenderWork, ISinglePassRenderWork)

const MStringId MVoxelizerRenderWork::VoxelizerBufferOutput = MStringId("Voxelizer Buffer Output");

class MVoxelMapPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
    explicit MVoxelMapPropertyDecorator(MVoxelizerRenderWork* pOwner)
        : m_owner(pOwner)
    {}

    void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override
    {
        MORTY_ASSERT(
                m_voxelParam = pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_VOXEL_MAP_DATA)
        );
        MORTY_ASSERT(
                m_rWVoxelTableParam = pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_VOXEL_TABLE)
        );
        MORTY_ASSERT(
                m_voxelGITextureParam =
                        pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::VOXELIZER_VOXEL_TEXTURE_NAME)
        );
    }

    void Update(const MRenderInfo& info) override
    {
        MORTY_UNUSED(info);

        if (m_voxelParam)
        {
            auto&            voxelMapInformation = m_voxelParam->var.GetValue<MVariantStruct>();

            MVoxelMapSetting setting = m_owner->GetVoxelSetting();
            auto&            settingStruct =
                    voxelMapInformation.GetVariant<MVariantStruct>(MShaderPropertyName::VOXEL_MAP_SETTING);
            MRenderInfo::FillVoxelMapSetting(setting, settingStruct);

            const MVoxelClipmap& clipmap = setting.vClipmap[setting.nClipmapIdx];

            const float          fVoxelTableSize = static_cast<float>(setting.nResolution) * clipmap.fVoxelSize;
            const auto           m4CameraProj    = MRenderSystem::MatrixOrthoOffCenterLH(
                    clipmap.f3VoxelOrigin.x,
                    clipmap.f3VoxelOrigin.x + fVoxelTableSize,
                    clipmap.f3VoxelOrigin.y + fVoxelTableSize,
                    clipmap.f3VoxelOrigin.y,
                    clipmap.f3VoxelOrigin.z,
                    clipmap.f3VoxelOrigin.z + fVoxelTableSize
            );

            voxelMapInformation.SetVariant<Matrix4>(MShaderPropertyName::VOXELIZER_CAMERA_PROJ_MATRIX, m4CameraProj);

            m_voxelParam->SetDirty();
        }

        m_rWVoxelTableParam->SetBuffer(m_owner->GetVoxelTableBuffer());
        m_voxelGITextureParam->SetTexture(m_owner->GetVoxelGITexture());
    }

    std::shared_ptr<MShaderConstantParam> m_voxelParam          = nullptr;
    std::shared_ptr<MShaderStorageParam>  m_rWVoxelTableParam   = nullptr;
    std::shared_ptr<MShaderTextureParam>  m_voxelGITextureParam = nullptr;

    MVoxelizerRenderWork*                 m_owner = nullptr;
};


void MVoxelizerRenderWork::Initialize(MEngine* pEngine)
{
    Super::Initialize(pEngine);

    InitializeBuffer();
    InitializeRenderPass();
    InitializeDispatcher();
    InitializeVoxelTextureDispatcher();

#if MORTY_VXGI_ENABLE
    m_framePropertyUpdateDecorator = std::make_shared<MVoxelMapPropertyDecorator>(this);
#endif
}

void MVoxelizerRenderWork::Release()
{
    ReleaseDispatcher();
    ReleaseBuffer();
    ReleaseVoxelTextureDispatcher();

    Super::Release();
}

std::shared_ptr<IShaderPropertyUpdateDecorator> MVoxelizerRenderWork::GetFramePropertyDecorator()
{
    return m_framePropertyUpdateDecorator;
}

void                      MVoxelizerRenderWork::Resize(Vector2i size) { MORTY_UNUSED(size); }

const MBuffer*            MVoxelizerRenderWork::GetVoxelTableBuffer() const { return &m_voxelizerBuffer; }

std::shared_ptr<MTexture> MVoxelizerRenderWork::GetVoxelGITexture() const { return m_voxelGITexture; }

MBoundsAABB               MVoxelizerRenderWork::GetVoxelizerBoundsAABB(uint32_t nClipmapIdx) const
{
    return MVoxelMapUtil::GetClipMapBounding(m_voxelSetting.vClipmap[nClipmapIdx]);
}

void MVoxelizerRenderWork::SetupVoxelSetting(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx)
{
    const uint32_t    nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;

    MVoxelMapSetting& voxelSetting = m_voxelSetting;
    voxelSetting.nResolution       = nVoxelTableSize;
    voxelSetting.nClipmapIdx       = nClipmapIdx;
    voxelSetting.nViewportSize     = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

    voxelSetting.vClipmap[nClipmapIdx] = MVoxelMapUtil::GetClipMap(f3CameraPosition, nClipmapIdx);
}

void MVoxelizerRenderWork::Render(const MRenderInfo& info)
{
    const MMeshManager*                      pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

    MCullingResultSpecificMaterialRenderable indirectMesh;
    indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
    indirectMesh.SetPropertyBlockAdapter({
            GetRenderGraph()->GetFrameProperty(),
    });
    indirectMesh.SetInstanceCulling(GetRenderGraph()->GetVoxelizerCullingResult());
    indirectMesh.SetMaterial(GetVoxelizerMaterial());

    std::unordered_map<MStringId, bool> tVoxelizerDefined = {{MRenderGlobal::SHADER_SKELETON_ENABLE, false}};

    indirectMesh.SetMaterialFilter(std::make_shared<MMaterialMacroDefineFilter>(tVoxelizerDefined));

    Render(info,
           {
                   &indirectMesh,
           });
}

void MVoxelizerRenderWork::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
    MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

    pCommand->ResetBuffer(&m_voxelizerBuffer);

    constexpr uint32_t fViewportSize = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);
    pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, fViewportSize, fViewportSize));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, fViewportSize, fViewportSize));

    for (IRenderable* pRenderable: vRenderable) { pRenderable->Render(pCommand); }

    pCommand->EndRenderPass();

    if (m_voxelizerVoxelMapSetting)
    {
        auto& settingStruct = m_voxelizerVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>(
                MShaderPropertyName::VOXEL_MAP_SETTING
        );
        MRenderInfo::FillVoxelMapSetting(m_voxelSetting, settingStruct);

        m_voxelizerVoxelMapSetting->SetDirty();
    }

    pCommand->AddBufferMemoryBarrier(
            {&m_voxelizerBuffer},
            MEBufferBarrierStage::EPixelShaderWrite,
            MEBufferBarrierStage::EComputeShaderRead
    );

    pCommand->DispatchComputeJob(
            m_voxelTextureGenerator,
            MRenderGlobal::VOXEL_TABLE_SIZE / 8,
            MRenderGlobal::VOXEL_TABLE_SIZE / 8,
            MRenderGlobal::VOXEL_TABLE_SIZE / 8
    );
}

void MVoxelizerRenderWork::InitializeBuffer()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    const int      nVoxelCubeMapSize =
            MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE;


    const int nVoxelTableMemorySize = sizeof(VoxelizerOutput) * nVoxelCubeMapSize;
    m_voxelizerBuffer               = MBuffer::CreateBuffer(
            MBuffer::MMemoryType::EDeviceLocal,
            MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect,
            "Voxelizer Voxel Table Buffer"
    );
    m_voxelizerBuffer.ReallocMemory(nVoxelTableMemorySize);
    m_voxelizerBuffer.DestroyBuffer(pRenderSystem->GetDevice());
    m_voxelizerBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nVoxelTableMemorySize);
}

void MVoxelizerRenderWork::ReleaseBuffer()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    ;
    m_voxelizerBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MVoxelizerRenderWork::InitializeDispatcher()
{
    MResourceSystem*             pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::shared_ptr<MResource>   voxelizerVS = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mvs");
    std::shared_ptr<MResource>   voxelizerPS = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mps");
    std::shared_ptr<MResource>   voxelizerGS = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mgs");

    const std::vector<MStringId> defines = {
            MRenderGlobal::DRAW_MESH_INSTANCING_NONE,
            MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM,
            MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE,
    };

    for (const MStringId& key: defines)
    {
        auto pVoxelizerMaterial = pResourceSystem->CreateResource<MMaterialTemplate>();
        pVoxelizerMaterial->SetCullMode(MECullMode::ECullNone);
        pVoxelizerMaterial->AddDefine(key, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);

#ifdef MORTY_WIN
        pVoxelizerMaterial->SetConservativeRasterizationEnable(true);
        pVoxelizerMaterial->AddDefine(
                MRenderGlobal::VOXELIZER_CONSERVATIVE_RASTERIZATION,
                MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG
        );
        pVoxelizerMaterial->LoadShader(voxelizerGS);
#endif

        pVoxelizerMaterial->LoadShader(voxelizerVS);
        pVoxelizerMaterial->LoadShader(voxelizerPS);

        m_voxelizerMaterial[key] = MMaterial::CreateMaterial(pVoxelizerMaterial);
    }
}

void MVoxelizerRenderWork::ReleaseDispatcher() { m_voxelizerMaterial.clear(); }

void MVoxelizerRenderWork::InitializeVoxelTextureDispatcher()
{
    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    MObjectSystem*       pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();

    m_voxelTextureGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
    m_voxelTextureGenerator->LoadComputeShader("Shader/Voxel/voxelizer.mcs");

    const std::shared_ptr<MShaderPropertyBlock>& params = m_voxelTextureGenerator->GetShaderPropertyBlock(0);
    MORTY_ASSERT(
            m_voxelizerVoxelMapSetting =
                    params->FindConstantParam(MShaderPropertyName::VOXELIZER_CBUFFER_VOXEL_MAP_NAME)
    );

    m_voxelGITexture = MTexture::CreateVXGIMap(Vector3i(
            MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_DIFFUSE_CONE_COUNT,
            MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM,
            MRenderGlobal::VOXEL_TABLE_SIZE
    ));

    m_voxelGITexture->GenerateBuffer(pRenderSystem->GetDevice());

    if (auto pVoxelTexture = params->FindTextureParam(MShaderPropertyName::VOXELIZER_VOXEL_TEXTURE_NAME))
    {
        pVoxelTexture->SetTexture(m_voxelGITexture);
    }

    if (auto pVoxelTable = params->FindStorageParam(MShaderPropertyName::VOXELIZER_VOXEL_TABLE_NAME))
    {
        pVoxelTable->SetBuffer(&m_voxelizerBuffer);
    }
}

void MVoxelizerRenderWork::ReleaseVoxelTextureDispatcher()
{
    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_voxelTextureGenerator->DeleteLater();
    m_voxelTextureGenerator = nullptr;

    m_voxelGITexture->DestroyBuffer(pRenderSystem->GetDevice());
    m_voxelGITexture = nullptr;
}


void MVoxelizerRenderWork::InitializeRenderPass()
{
#if MORTY_DEBUG
    m_renderPass.m_strDebugName = "Voxelizer";
#endif

    m_renderPass.SetDepthTestEnable(false);
    m_renderPass.SetDepthWriteEnable(false);
    m_renderPass.SetViewportNum(1);
}

void MVoxelizerRenderWork::BindTarget()
{
    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTarget());
}

std::vector<MRenderTaskInputDesc> MVoxelizerRenderWork::InitInputDesc()
{
    return {
            {MShadowMapRenderWork::ShadowMapBufferOutput, METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MVoxelizerRenderWork::InitOutputDesc()
{
    return {
            {VoxelizerBufferOutput, {true, MColor::Black_T}},
    };
}
