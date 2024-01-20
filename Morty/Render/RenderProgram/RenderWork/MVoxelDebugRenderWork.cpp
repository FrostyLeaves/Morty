#include "MVoxelDebugRenderWork.h"

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
#include "RenderProgram/MeshRender/MIndirectIndexRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Variant/MVariant.h"
#include "VXGI/MVoxelMapUtil.h"

MORTY_CLASS_IMPLEMENT(MVoxelDebugRenderWork, ISinglePassRenderWork)


const MStringId MVoxelDebugRenderWork::BackBufferOutput = MStringId("Voxel Debug Back Buffer Output");
const MStringId MVoxelDebugRenderWork::DepthBufferOutput = MStringId("Voxel Debug Depth Buffer Output");



void MVoxelDebugRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeBuffer();
	InitializeDispatcher();


	m_renderPass.SetDepthTestEnable(true);
	m_renderPass.SetDepthWriteEnable(true);
}

void MVoxelDebugRenderWork::Release()
{
	ReleaseDispatcher();
	ReleaseBuffer();

	Super::Release();
}

std::shared_ptr<IShaderPropertyUpdateDecorator> MVoxelDebugRenderWork::GetFramePropertyDecorator()
{
	return m_pFramePropertyUpdateDecorator;
}

const MBuffer* MVoxelDebugRenderWork::GetVoxelDebugBuffer() const
{
	return &m_drawIndirectBuffer;
}

void MVoxelDebugRenderWork::Render(const MRenderInfo& info)
{
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	auto pVoxelizerWork = GetInput(0)->GetLinkedNode()->DynamicCast<MVoxelizerRenderWork>();

	MIndirectIndexRenderable debugRender;
	debugRender.SetMaterial(GetVoxelDebugMaterial());
	debugRender.SetPropertyBlockAdapter({
		GetRenderGraph()->GetFrameProperty()
		});
	debugRender.SetIndirectIndexBuffer(GetVoxelDebugBuffer());
	debugRender.SetMeshBuffer(pMeshManager->GetMeshBuffer());

	Render(info, pVoxelizerWork->GetVoxelSetting(), pVoxelizerWork->GetVoxelTableBuffer(), {
	   &debugRender,
	});
}

void MVoxelDebugRenderWork::Render(const MRenderInfo& info, const MVoxelMapSetting& voxelSetting, const MBuffer* pVoxelizerBuffer, const std::vector<IRenderable*>& vRenderable)
{
	const int nDebugClipmapIdx = 2;

	if (nDebugClipmapIdx != voxelSetting.nClipmapIdx)
	{
		return;
	}

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	if (m_pDebugVoxelMapSetting)
	{
		auto& settingStruct = m_pDebugVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>(MShaderPropertyName::VOXEL_MAP_SETTING);
		MRenderInfo::FillVoxelMapSetting(voxelSetting, settingStruct);

		m_pDebugVoxelMapSetting->SetDirty();
	}

	if (m_pVoxelStorageBuffer)
	{
		m_pVoxelStorageBuffer->SetBuffer(pVoxelizerBuffer);
	}

	pCommand->AddBufferMemoryBarrier(
		{ &m_drawIndirectBuffer },
		MEBufferBarrierStage::EDrawIndirectRead,
		MEBufferBarrierStage::EComputeShaderWrite
	);

	pCommand->DispatchComputeJob(m_pVoxelDebugIndirectGenerator
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8
		, MRenderGlobal::VOXEL_TABLE_SIZE / 8);

	pCommand->AddBufferMemoryBarrier(
		{ &m_drawIndirectBuffer },
		MEBufferBarrierStage::EComputeShaderWrite,
		MEBufferBarrierStage::EDrawIndirectRead
	);

	pCommand->AddBufferMemoryBarrier(
		{ pVoxelizerBuffer },
		MEBufferBarrierStage::EComputeShaderRead,
		MEBufferBarrierStage::EPixelShaderWrite
	);

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

void MVoxelDebugRenderWork::InitializeBuffer()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	const int nVoxelCubeMapSize = MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE * MRenderGlobal::VOXEL_TABLE_SIZE;

	const size_t nDrawIndirectBufferSize = sizeof(MDrawIndexedIndirectData) * nVoxelCubeMapSize;
	m_drawIndirectBuffer = MBuffer::CreateIndirectDrawBuffer("Voxel Draw Indirect Buffer");
	m_drawIndirectBuffer.ReallocMemory(nDrawIndirectBufferSize);
	m_drawIndirectBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nDrawIndirectBufferSize);
}

void MVoxelDebugRenderWork::ReleaseBuffer()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_drawIndirectBuffer.DestroyBuffer(pRenderSystem->GetDevice());
}

void MVoxelDebugRenderWork::InitializeDispatcher()
{
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();


	m_pVoxelDebugIndirectGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVoxelDebugIndirectGenerator->LoadComputeShader("Shader/Voxel/voxel_debug_view.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVoxelDebugIndirectGenerator->GetShaderPropertyBlock(0);
	MORTY_ASSERT(m_pDebugVoxelMapSetting = params->FindConstantParam(MShaderPropertyName::VOXELIZER_CBUFFER_VOXEL_MAP_NAME));
	MORTY_ASSERT(m_pVoxelStorageBuffer = params->FindStorageParam(MShaderPropertyName::VOXELIZER_VOXEL_TABLE_NAME));

	const auto pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	const auto cubeMesh = pMeshManager->GetCubeMesh();
	params->SetValue(MShaderPropertyName::VOXELIZER_CUBE_MESH_INDEX, static_cast<uint32_t>(cubeMesh.indexMemoryInfo.begin / sizeof(uint32_t)));
	params->SetValue(MShaderPropertyName::VOXELIZER_CUBE_MESH_COUNT, static_cast<uint32_t>(cubeMesh.indexMemoryInfo.size / sizeof(uint32_t)));

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

	m_pVoxelDebugMaterial = MMaterial::CreateMaterial(pVoxelDebugTemplate);
}

void MVoxelDebugRenderWork::ReleaseDispatcher()
{
	m_pVoxelDebugIndirectGenerator->DeleteLater();
	m_pVoxelDebugIndirectGenerator = nullptr;

	m_pVoxelDebugMaterial = nullptr;
}

void MVoxelDebugRenderWork::BindTarget()
{
	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTarget());
}

std::vector<MStringId> MVoxelDebugRenderWork::GetInputName()
{
	return {
		MVoxelizerRenderWork::VoxelizerBufferOutput,
	};
}

std::vector<MRenderTaskOutputDesc> MVoxelDebugRenderWork::GetOutputName()
{
	return {
		{ BackBufferOutput, {true, MColor::Black_T }},
		{ DepthBufferOutput, {true, MColor::Black_T }}
	};
}
