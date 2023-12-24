#include "MVoxelizerRenderWork.h"

#include "Render/MRenderGlobal.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Material/MComputeDispatcher.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"
#include "Variant/MVariant.h"
#include "VXGI/MVoxelMapUtil.h"

MORTY_CLASS_IMPLEMENT(MVoxelizerRenderWork, ISinglePassRenderWork)


class MVoxelMapPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
	explicit MVoxelMapPropertyDecorator(MVoxelizerRenderWork* pOwner):m_pOwner(pOwner){}

	void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override
	{
		MORTY_ASSERT(m_pVoxelParam = pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_VOXEL_MAP_DATA));
		MORTY_ASSERT(m_pRWVoxelTableParam = pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_VOXEL_TABLE));
		MORTY_ASSERT(m_pVoxelGITextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::VOXELIZER_VOXEL_TEXTURE_NAME));
	}
	void Update(const MRenderInfo& info) override
	{
		MORTY_UNUSED(info);

		if (m_pVoxelParam)
		{
			auto& voxelMapInformation = m_pVoxelParam->var.GetValue<MVariantStruct>();

			MVoxelMapSetting setting = m_pOwner->GetVoxelSetting();
			auto& settingStruct = voxelMapInformation.GetVariant<MVariantStruct>(MShaderPropertyName::VOXEL_MAP_SETTING);
			MRenderInfo::FillVoxelMapSetting(setting, settingStruct);

			const MVoxelClipmap& clipmap = setting.vClipmap[setting.nClipmapIdx];

			const float fVoxelTableSize = static_cast<float>(setting.nResolution) * clipmap.fVoxelSize;
			const auto m4CameraProj = MRenderSystem::MatrixOrthoOffCenterLH(
				clipmap.f3VoxelOrigin.x,
				clipmap.f3VoxelOrigin.x + fVoxelTableSize,
				clipmap.f3VoxelOrigin.y + fVoxelTableSize,
				clipmap.f3VoxelOrigin.y,
				clipmap.f3VoxelOrigin.z,
				clipmap.f3VoxelOrigin.z + fVoxelTableSize
			);

			voxelMapInformation.SetVariant<Matrix4>(MShaderPropertyName::VOXELIZER_CAMERA_PROJ_MATRIX, m4CameraProj);

			m_pVoxelParam->SetDirty();

		}

		m_pRWVoxelTableParam->SetBuffer(m_pOwner->GetVoxelTableBuffer());
		m_pVoxelGITextureParam->SetTexture(m_pOwner->GetVoxelGITexture());
	}

	std::shared_ptr<MShaderConstantParam> m_pVoxelParam = nullptr;
	std::shared_ptr<MShaderStorageParam> m_pRWVoxelTableParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pVoxelGITextureParam = nullptr;

	MVoxelizerRenderWork* m_pOwner = nullptr;
};


void MVoxelizerRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeBuffer();
	InitializeRenderPass();
	InitializeDispatcher();
	InitializeVoxelTextureDispatcher();

#if MORTY_VXGI_ENABLE
	m_pFramePropertyUpdateDecorator = std::make_shared<MVoxelMapPropertyDecorator>(this);
#endif
}

void MVoxelizerRenderWork::Release(MEngine* pEngine)
{
	ReleaseDispatcher();
	ReleaseRenderPass();
	ReleaseBuffer();
	ReleaseVoxelTextureDispatcher();

	Super::Release(pEngine);
}

std::shared_ptr<IShaderPropertyUpdateDecorator> MVoxelizerRenderWork::GetFramePropertyDecorator()
{
	return m_pFramePropertyUpdateDecorator;
}

void MVoxelizerRenderWork::Resize(Vector2i size)
{
	MORTY_UNUSED(size);
}

const MBuffer* MVoxelizerRenderWork::GetVoxelTableBuffer() const
{
	return &m_voxelizerBuffer;
}

std::shared_ptr<MTexture> MVoxelizerRenderWork::GetVoxelGITexture() const
{
	return m_voxelGITexture;
}

MBoundsAABB MVoxelizerRenderWork::GetVoxelizerBoundsAABB(uint32_t nClipmapIdx) const
{
	return MVoxelMapUtil::GetClipMapBounding(m_voxelSetting.vClipmap[nClipmapIdx]);
}

void MVoxelizerRenderWork::SetupVoxelSetting(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx)
{
	const uint32_t nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;

	MVoxelMapSetting& voxelSetting = m_voxelSetting;
	voxelSetting.nResolution = nVoxelTableSize;
	voxelSetting.nClipmapIdx = nClipmapIdx;
	voxelSetting.nViewportSize = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

	voxelSetting.vClipmap[nClipmapIdx] = MVoxelMapUtil::GetClipMap(f3CameraPosition, nClipmapIdx);
}

void MVoxelizerRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	pCommand->ResetBuffer(&m_voxelizerBuffer);

	constexpr uint32_t fViewportSize = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

	pCommand->BeginRenderPass(&m_renderPass);
	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, fViewportSize, fViewportSize));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, fViewportSize, fViewportSize));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();

	if (m_pVoxelizerVoxelMapSetting)
	{
		auto& settingStruct = m_pVoxelizerVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>(MShaderPropertyName::VOXEL_MAP_SETTING);
		MRenderInfo::FillVoxelMapSetting(m_voxelSetting, settingStruct);

		m_pVoxelizerVoxelMapSetting->SetDirty();
	}

	pCommand->AddBufferMemoryBarrier(
		{ &m_voxelizerBuffer },
		MEBufferBarrierStage::EPixelShaderWrite,
		MEBufferBarrierStage::EComputeShaderRead
	);

	pCommand->DispatchComputeJob(m_pVoxelTextureGenerator
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8);

}

void MVoxelizerRenderWork::InitializeBuffer()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	const int nVoxelCubeMapSize = MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE;


	const int nVoxelTableMemorySize = sizeof(VoxelizerOutput) * nVoxelCubeMapSize;
	m_voxelizerBuffer = MBuffer::CreateBuffer(MBuffer::MMemoryType::EDeviceLocal
		, MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect
		, "Voxelizer Voxel Table Buffer");
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
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> voxelizerVS = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mvs");
	std::shared_ptr<MResource> voxelizerPS = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mps");
	std::shared_ptr<MResource> voxelizerGS = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mgs");

	const std::vector<MStringId> defines = {
		MRenderGlobal::DRAW_MESH_INSTANCING_NONE,
		MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM,
		MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE,
	};

	for (const MStringId& key : defines)
	{
		auto pVoxelizerMaterial = pResourceSystem->CreateResource<MMaterialTemplate>();
		pVoxelizerMaterial->SetCullMode(MECullMode::ECullNone);
		pVoxelizerMaterial->AddDefine(key, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
		
#ifdef MORTY_WIN
		pVoxelizerMaterial->SetConservativeRasterizationEnable(true);
		pVoxelizerMaterial->AddDefine(MRenderGlobal::VOXELIZER_CONSERVATIVE_RASTERIZATION, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
		pVoxelizerMaterial->LoadShader(voxelizerGS);
#endif

		pVoxelizerMaterial->LoadShader(voxelizerVS);
		pVoxelizerMaterial->LoadShader(voxelizerPS);   
		
		m_tVoxelizerMaterial[key] = MMaterial::CreateMaterial(pVoxelizerMaterial);
	}
}

void MVoxelizerRenderWork::ReleaseDispatcher()
{
	m_tVoxelizerMaterial.clear();
}

void MVoxelizerRenderWork::InitializeVoxelTextureDispatcher()
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();

	m_pVoxelTextureGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVoxelTextureGenerator->LoadComputeShader("Shader/Voxel/voxelizer.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVoxelTextureGenerator->GetShaderPropertyBlock(0);
	MORTY_ASSERT(m_pVoxelizerVoxelMapSetting = params->FindConstantParam(MShaderPropertyName::VOXELIZER_CBUFFER_VOXEL_MAP_NAME));

	m_voxelGITexture = MTexture::CreateVXGIMap();
	m_voxelGITexture->SetSize(Vector3i(	  MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_DIFFUSE_CONE_COUNT
		                                , MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM
		                                , MRenderGlobal::VOXEL_TABLE_SIZE));
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

	m_pVoxelTextureGenerator->DeleteLater();
	m_pVoxelTextureGenerator = nullptr;

	m_voxelGITexture->DestroyBuffer(pRenderSystem->GetDevice());
	m_voxelGITexture = nullptr;
}


void MVoxelizerRenderWork::InitializeRenderPass()
{
#if MORTY_DEBUG
	m_renderPass.m_strDebugName = "Voxelizer";
#endif

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pVoxelizerRenderTarget = MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8);
	m_pVoxelizerRenderTarget->SetName("Voxelizer Render Target");
	int nRenderTargetSize = static_cast<int>(MRenderGlobal::VOXEL_VIEWPORT_SIZE);
	m_pVoxelizerRenderTarget->SetSize({ nRenderTargetSize, nRenderTargetSize });
	m_pVoxelizerRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
	SetRenderTarget({
		{ {m_pVoxelizerRenderTarget, {true, MColor::Black_T}} },
		{},
		{}
		});

	m_renderPass.SetDepthTestEnable(false);
	m_renderPass.SetDepthWriteEnable(false);
	m_renderPass.SetViewportNum(1);
}

void MVoxelizerRenderWork::ReleaseRenderPass()
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pVoxelizerRenderTarget->DestroyBuffer(pRenderSystem->GetDevice());

	m_pVoxelizerRenderTarget = nullptr;
}
