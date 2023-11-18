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


		if (m_pRWVoxelTableParam->pBuffer != m_pOwner->GetVoxelTableBuffer())
		{
			m_pRWVoxelTableParam->pBuffer = m_pOwner->GetVoxelTableBuffer();
			m_pRWVoxelTableParam->SetDirty();
		}

		if (m_pVoxelGITextureParam->pTexture != m_pOwner->GetVoxelGITexture())
		{
			m_pVoxelGITextureParam->pTexture = m_pOwner->GetVoxelGITexture();
			m_pVoxelGITextureParam->SetDirty();
		}
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

	m_pFramePropertyUpdateDecorator = std::make_shared<MVoxelMapPropertyDecorator>(this);
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

const MBuffer* MVoxelizerRenderWork::GetVoxelTableBuffer() const
{
	return &m_voxelizerBuffer;
}

const MBuffer* MVoxelizerRenderWork::GetVoxelDebugBuffer() const
{
	return &m_drawIndirectBuffer;
}

std::shared_ptr<MTexture> MVoxelizerRenderWork::GetVoxelGITexture() const
{
	return m_voxelGITexture;
}

MBoundsAABB MVoxelizerRenderWork::GetVoxelizerBoundsAABB(uint32_t nClipmapIdx) const
{
	return MBoundsAABB(
		m_voxelSetting.vClipmap[nClipmapIdx].f3VoxelOrigin,
		m_voxelSetting.vClipmap[nClipmapIdx].f3VoxelOrigin +
		m_voxelSetting.nResolution *
		m_voxelSetting.vClipmap[nClipmapIdx].fVoxelSize
	);
}

void MVoxelizerRenderWork::SetupVoxelSetting(const Vector3& f3CameraPosition, const uint32_t nClipmapIdx)
{
	const uint32_t nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;
	const float fBasicVoxelSize = MRenderGlobal::VOXEL_BASIC_VOXEL_SIZE;

	MVoxelMapSetting& voxelSetting = m_voxelSetting;
	voxelSetting.nResolution = nVoxelTableSize;
	voxelSetting.nClipmapIdx = nClipmapIdx;
	voxelSetting.nViewportSize = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

	const float fVoxelSize = fBasicVoxelSize * std::powf(2.0f, nClipmapIdx);
	Vector3 f3Origin = f3CameraPosition - nVoxelTableSize * fVoxelSize * 0.5f;
	const Vector3i n3FloorPosition = MMath::Floor(f3Origin / fVoxelSize);
	f3Origin = Vector3(n3FloorPosition.x, n3FloorPosition.y, n3FloorPosition.z) * fVoxelSize;

	voxelSetting.vClipmap[nClipmapIdx] =
	{
		f3Origin,
		fVoxelSize
	};
}

void MVoxelizerRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	pCommand->ResetBuffer(&m_voxelizerBuffer);

	pCommand->AddBufferMemoryBarrier(
		{ &m_voxelizerBuffer },
		MEBufferBarrierStage::EPixelShaderRead,
		MEBufferBarrierStage::EPixelShaderWrite
	);

	constexpr uint32_t fViewportSize = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

	pCommand->BeginRenderPass(&m_voxelizerRenderPass);
	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, fViewportSize, fViewportSize));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, fViewportSize, fViewportSize));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();

	if (m_pDebugVoxelMapSetting)
	{
		auto& settingStruct = m_pDebugVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>(MShaderPropertyName::VOXEL_MAP_SETTING);
		MRenderInfo::FillVoxelMapSetting(m_voxelSetting, settingStruct);

		m_pDebugVoxelMapSetting->SetDirty();
	}

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

	pCommand->AddBufferMemoryBarrier(
		{ &m_drawIndirectBuffer },
		MEBufferBarrierStage::EDrawIndirectRead,
		MEBufferBarrierStage::EComputeShaderWrite
	);

	//
	pCommand->DispatchComputeJob(m_pVoxelDebugIndirectGenerator
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8);

	{
		pCommand->DispatchComputeJob(m_pVoxelTextureGenerator
			, MRenderGlobal::VOXEL_TABLE_SIZE / 8
			, MRenderGlobal::VOXEL_TABLE_SIZE / 8
			, MRenderGlobal::VOXEL_TABLE_SIZE / 8);
	}

	pCommand->AddBufferMemoryBarrier(
		{ &m_drawIndirectBuffer },
		MEBufferBarrierStage::EComputeShaderWrite,
		MEBufferBarrierStage::EDrawIndirectRead
	);

	pCommand->AddBufferMemoryBarrier(
		{ &m_voxelizerBuffer },
		MEBufferBarrierStage::EComputeShaderRead,
		MEBufferBarrierStage::EPixelShaderRead
	);
}

void MVoxelizerRenderWork::RenderDebugVoxel(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	const Vector2i v2LeftTop = info.f2ViewportLeftTop;
	const Vector2i v2Size = info.f2ViewportSize;

	pCommand->BeginRenderPass(&m_renderPass);
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();
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


	const size_t nDrawIndirectBufferSize = sizeof(MDrawIndexedIndirectData) * nVoxelCubeMapSize;
	m_drawIndirectBuffer = MBuffer::CreateIndirectDrawBuffer("Voxel Draw Indirect Buffer");
	m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
	m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);
}

void MVoxelizerRenderWork::ReleaseBuffer()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_voxelizerBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MVoxelizerRenderWork::InitializeDispatcher()
{
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();


	m_pVoxelDebugIndirectGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVoxelDebugIndirectGenerator->LoadComputeShader("Shader/Voxel/voxel_debug_view.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVoxelDebugIndirectGenerator->GetShaderPropertyBlock(0);
	MORTY_ASSERT(m_pDebugVoxelMapSetting = params->FindConstantParam(MShaderPropertyName::VOXELIZER_CBUFFER_VOXEL_MAP_NAME));

	auto pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	auto cubeMesh = pMeshManager->GetCubeMesh();
	params->SetValue(MShaderPropertyName::VOXELIZER_CUBE_MESH_INDEX, static_cast<uint32_t>(cubeMesh.indexMemoryInfo.begin / sizeof(uint32_t)));
	params->SetValue(MShaderPropertyName::VOXELIZER_CUBE_MESH_COUNT, static_cast<uint32_t>(cubeMesh.indexMemoryInfo.size / sizeof(uint32_t)));

	if (auto pIndirectDraws = params->FindStorageParam(MShaderPropertyName::CULLING_OUTPUT_DRAW_DATA))
	{
		pIndirectDraws->pBuffer = &m_drawIndirectBuffer;
		pIndirectDraws->SetDirty();
	}

	if (auto pVoxelTable = params->FindStorageParam(MShaderPropertyName::VOXELIZER_VOXEL_TABLE_NAME))
	{
		pVoxelTable->pBuffer = &m_voxelizerBuffer;
		pVoxelTable->SetDirty();
	}

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
		auto pVoxelizerMaterial = pResourceSystem->CreateResource<MMaterialResource>();
		pVoxelizerMaterial->SetCullMode(MECullMode::ECullNone);
		pVoxelizerMaterial->GetShaderMacro().AddUnionMacro(key, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
		pVoxelizerMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::VOXELIZER_CONSERVATIVE_RASTERIZATION, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
		pVoxelizerMaterial->SetConservativeRasterizationEnable(true);
		pVoxelizerMaterial->LoadShader(voxelizerVS);
		pVoxelizerMaterial->LoadShader(voxelizerPS);   
		pVoxelizerMaterial->LoadShader(voxelizerGS);
		
		m_tVoxelizerMaterial[key] = pVoxelizerMaterial;
	}

	


	std::shared_ptr<MResource> voxelDebugVS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mvs");
	std::shared_ptr<MResource> voxelDebugPS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mps");
	m_pVoxelDebugMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pVoxelDebugMaterial->SetCullMode(MECullMode::ECullBack);
	m_pVoxelDebugMaterial->SetMaterialType(MEMaterialType::ECustom);
	m_pVoxelDebugMaterial->LoadShader(voxelDebugVS);
	m_pVoxelDebugMaterial->LoadShader(voxelDebugPS);
}

void MVoxelizerRenderWork::ReleaseDispatcher()
{
	m_pVoxelDebugIndirectGenerator->DeleteLater();
	m_pVoxelDebugIndirectGenerator = nullptr;

	m_tVoxelizerMaterial.clear();
	m_pVoxelDebugMaterial = nullptr;
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
		pVoxelTexture->pTexture = m_voxelGITexture;
		pVoxelTexture->SetDirty();
	}

	if (auto pVoxelTable = params->FindStorageParam(MShaderPropertyName::VOXELIZER_VOXEL_TABLE_NAME))
	{
		pVoxelTable->pBuffer = &m_voxelizerBuffer;
		pVoxelTable->SetDirty();
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
	m_voxelizerRenderPass.m_strDebugName = "Voxelizer";
#endif

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pVoxelizerRenderTarget = MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8);
	m_pVoxelizerRenderTarget->SetName("Voxelizer Render Target");
	m_pVoxelizerRenderTarget->SetSize({ MRenderGlobal::VOXEL_VIEWPORT_SIZE, MRenderGlobal::VOXEL_VIEWPORT_SIZE });
	m_pVoxelizerRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());

	m_voxelizerRenderPass.AddBackTexture(m_pVoxelizerRenderTarget, { true, false, MColor::Black_T });
	m_voxelizerRenderPass.SetDepthTestEnable(false);
	m_voxelizerRenderPass.SetDepthWriteEnable(false);
	m_voxelizerRenderPass.SetViewportNum(1);
	m_voxelizerRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

}

void MVoxelizerRenderWork::ReleaseRenderPass()
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_voxelizerRenderPass.DestroyBuffer(pRenderSystem->GetDevice());

	m_pVoxelizerRenderTarget->DestroyBuffer(pRenderSystem->GetDevice());
	m_pVoxelizerRenderTarget = nullptr;
}
