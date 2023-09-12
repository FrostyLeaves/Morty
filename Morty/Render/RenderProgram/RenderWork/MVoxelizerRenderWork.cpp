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

	m_renderPass.SetDepthTestEnable(false);
	m_renderPass.SetDepthWriteEnable(false);
}

void MVoxelizerRenderWork::Release(MEngine* pEngine)
{
	Super::Release(pEngine);
}

void MVoxelizerRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	pCommand->BeginRenderPass(&m_renderPass);
	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, MRenderGlobal::VOXEL_TABLE_SIZE, MRenderGlobal::VOXEL_TABLE_SIZE));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, MRenderGlobal::VOXEL_TABLE_SIZE, MRenderGlobal::VOXEL_TABLE_SIZE));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();


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


void MVoxelizerRenderWork::InitializeDispatcher()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();


	m_pVoxelMapGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVoxelMapGenerator->LoadComputeShader("Shader/Voxel/voxel_build_map.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVoxelMapGenerator->GetShaderPropertyBlocks()[0];

	if (auto setting = params->FindConstantParam("voxelMapSetting"))
	{
		auto& voxelMapSetting = setting->var.GetValue<MVariantStruct>();
		voxelMapSetting.SetVariant("f3VoxelOrigin", Vector3(0, 0, 0));
		voxelMapSetting.SetVariant("fResolution", float(MRenderGlobal::VOXEL_TABLE_SIZE));
		voxelMapSetting.SetVariant("fVoxelStep", 1.0f);
		setting->SetDirty();
	}

	auto pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	auto cubeMesh = pMeshManager->GetCubeMesh();
	params->SetValue("cubeMeshIndex", cubeMesh.indexInfo.begin);
	params->SetValue("cubeMeshCount", cubeMesh.indexInfo.size);

	if (auto pIndirectDraws = params->FindStorageParam("indirectDraws"))
	{
		const size_t nDrawIndirectBufferSize = sizeof(MDrawIndexedIndirectData)
			* MRenderGlobal::VOXEL_TABLE_SIZE
			* MRenderGlobal::VOXEL_TABLE_SIZE
			* MRenderGlobal::VOXEL_TABLE_SIZE;
		
		m_drawIndirectBuffer = MBuffer::CreateIndirectDrawBuffer("Voxel Draw Indirect Buffer");
		m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
		m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);

		pIndirectDraws->pBuffer = &m_drawIndirectBuffer;
		pIndirectDraws->SetDirty();
	}

	std::shared_ptr<MResource> voxelDebugVS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mvs");
	std::shared_ptr<MResource> voxelDebugPS = pResourceSystem->LoadResource("Shader/Voxel/voxel_debug_view.mps");
	m_pVoxelDebugMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pVoxelDebugMaterial->SetCullMode(MECullMode::ECullBack);
	m_pVoxelDebugMaterial->SetMaterialType(MEMaterialType::ECustom);
	m_pVoxelDebugMaterial->LoadVertexShader(voxelDebugVS);
	m_pVoxelDebugMaterial->LoadPixelShader(voxelDebugPS);

}

void MVoxelizerRenderWork::ReleaseDispatcher()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());

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