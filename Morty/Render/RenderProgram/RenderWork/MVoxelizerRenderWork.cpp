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
#include "Variant/MVariant.h"

MORTY_CLASS_IMPLEMENT(MVoxelizerRenderWork, ISinglePassRenderWork)


void MVoxelizerRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeBuffer();
	InitializeRenderPass();
	InitializeDispatcher();
}

void MVoxelizerRenderWork::Release(MEngine* pEngine)
{
	ReleaseDispatcher();
	ReleaseRenderPass();
	ReleaseBuffer();

	Super::Release(pEngine);
}

const MBuffer* MVoxelizerRenderWork::GetVoxelTableBuffer() const
{
	return &m_voxelizerBuffer;
}

void MVoxelizerRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	pCommand->BeginRenderPass(&m_voxelizerRenderPass);
	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, MRenderGlobal::VOXEL_TABLE_SIZE, MRenderGlobal::VOXEL_TABLE_SIZE));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, MRenderGlobal::VOXEL_TABLE_SIZE, MRenderGlobal::VOXEL_TABLE_SIZE));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();

	pCommand->AddGraphToComputeBarrier({
		&m_voxelizerBuffer
	});

	pCommand->DispatchComputeJob(m_pVoxelMapGenerator
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8);

	
	if (m_bDebugMode)
	{
		MViewport* pViewport = info.pViewport;
		Vector2 v2LeftTop = pViewport->GetLeftTop();
		Vector2 v2Size = pViewport->GetSize();

		pCommand->BeginRenderPass(&m_renderPass);
		pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
		pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));
		
		DrawVoxelizerMap(pCommand);

		pCommand->EndRenderPass();
	}

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


	m_pVoxelMapGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVoxelMapGenerator->LoadComputeShader("Shader/Voxel/voxel_build_map.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVoxelMapGenerator->GetShaderPropertyBlocks()[0];

	if (auto m_pVoxelMapSetting = params->FindConstantParam("cbVoxelMap"))
	{
		auto& settingStruct = m_pVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>("voxelMapSetting");

		settingStruct.SetVariant("f3VoxelOrigin", Vector3(0, 0, 0));
		settingStruct.SetVariant("fResolution", float(MRenderGlobal::VOXEL_TABLE_SIZE));
		settingStruct.SetVariant("fVoxelStep", 1.0f);
		m_pVoxelMapSetting->SetDirty();
	}

	auto pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	auto cubeMesh = pMeshManager->GetCubeMesh();
	params->SetValue("cubeMeshIndex", cubeMesh.indexInfo.begin);
	params->SetValue("cubeMeshCount", cubeMesh.indexInfo.size);

	if (auto pIndirectDraws = params->FindStorageParam("indirectDraws"))
	{
		pIndirectDraws->pBuffer = &m_drawIndirectBuffer;
		pIndirectDraws->SetDirty();
	}

	if (auto pVoxelTable = params->FindStorageParam("rVoxelTable"))
	{
		pVoxelTable->pBuffer = &m_drawIndirectBuffer;
		pVoxelTable->SetDirty();
	}

	std::shared_ptr<MResource> voxelDebugVS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mvs");
	std::shared_ptr<MResource> voxelDebugPS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mps");
	m_pVoxelDebugMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pVoxelDebugMaterial->SetCullMode(MECullMode::ECullBack);
	m_pVoxelDebugMaterial->SetMaterialType(MEMaterialType::ECustom);
	m_pVoxelDebugMaterial->LoadVertexShader(voxelDebugVS);
	m_pVoxelDebugMaterial->LoadPixelShader(voxelDebugPS);

	{
		const std::shared_ptr<MShaderPropertyBlock>& params = m_pVoxelDebugMaterial->GetShaderPropertyBlocks()[0];

		if (auto m_pVoxelMapSetting = params->FindConstantParam("cbVoxelMap"))
		{
			auto& settingStruct = m_pVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>("voxelMapSetting");

			settingStruct.SetVariant("f3VoxelOrigin", Vector3(0, 0, 0));
			settingStruct.SetVariant("fResolution", float(MRenderGlobal::VOXEL_TABLE_SIZE));
			settingStruct.SetVariant("fVoxelStep", 1.0f);
			m_pVoxelMapSetting->SetDirty();
		}
	}

}

void MVoxelizerRenderWork::ReleaseDispatcher()
{
	m_pVoxelMapGenerator->DeleteLater();
	m_pVoxelMapGenerator = nullptr;

	m_pVoxelDebugMaterial = nullptr;
}

void MVoxelizerRenderWork::DrawVoxelizerMap(MIRenderCommand* pCommand)
{
	//TODO set fream data.
	pCommand->SetUseMaterial(m_pVoxelDebugMaterial);

	auto pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	pCommand->DrawIndexedIndirect(
		pMeshManager->GetVertexBuffer(),
		pMeshManager->GetIndexBuffer(),
		&m_drawIndirectBuffer,
		0,
		m_drawIndirectBuffer.GetSize() / sizeof(MDrawIndexedIndirectData)
	);
}

void MVoxelizerRenderWork::InitializeRenderPass()
{
#if MORTY_DEBUG
	m_voxelizerRenderPass.m_strDebugName = "Voxelizer";
#endif

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_voxelizerRenderPass.SetViewportNum(1);
	m_voxelizerRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

}

void MVoxelizerRenderWork::ReleaseRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_voxelizerRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}
