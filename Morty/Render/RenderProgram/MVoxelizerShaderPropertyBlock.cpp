#include "MVoxelizerShaderPropertyBlock.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MShader.h"
#include "Basic/MViewport.h"
#include "Render/MVertex.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Material/MMaterial.h"
#include "Resource/MMaterialResource.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Variant/MVariant.h"


void MVoxelizerShaderPropertyBlock::Initialize(MEngine* pEngine)
{
	MForwardRenderShaderPropertyBlock::Initialize(pEngine);

	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	
	m_rwVoxelTableBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
	m_rwVoxelTableBuffer.m_eUsageType = MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect;

#if MORTY_DEBUG
	m_rwVoxelTableBuffer.m_strDebugBufferName = "Voxelizer Voxel Table Buffer";
#endif

	const int nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;
	const int nVoxelTableMemorySize = sizeof(VoxelizerOutput) * nVoxelTableSize * nVoxelTableSize * nVoxelTableSize;
	m_rwVoxelTableBuffer.ReallocMemory(nVoxelTableMemorySize);
	m_rwVoxelTableBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_rwVoxelTableBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, nVoxelTableMemorySize);
	
}

void MVoxelizerShaderPropertyBlock::Release(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_rwVoxelTableBuffer.DestroyBuffer(pRenderSystem->GetDevice());

	MForwardRenderShaderPropertyBlock::Release(pEngine);
}

std::shared_ptr<MMaterial> MVoxelizerShaderPropertyBlock::LoadMaterial(MEngine* pEngine) const
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mvs");
	std::shared_ptr<MResource> ps = pResourceSystem->LoadResource("Shader/Voxel/voxelizer.mps");
	auto pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->SetCullMode(MECullMode::ECullNone);
	pMaterial->LoadVertexShader(vs);
	pMaterial->LoadPixelShader(ps);

	return pMaterial;
}

void MVoxelizerShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MForwardRenderShaderPropertyBlock::BindMaterial(pMaterial);

	MORTY_ASSERT(m_pRWVoxelTableParam = m_pShaderPropertyBlock->FindStorageParam("rwVoxelTable"));
	MORTY_ASSERT(m_pVoxelMapSetting = m_pShaderPropertyBlock->FindConstantParam("cbVoxelMap"));

	m_pRWVoxelTableParam->pBuffer = &m_rwVoxelTableBuffer;
	m_pRWVoxelTableParam->SetDirty();
}


void MVoxelizerShaderPropertyBlock::SetVoxelMapSetting(const MVoxelMapSetting setting)
{
	if (!m_pVoxelMapSetting)
	{
		return;
	}

	auto& settingStruct = m_pVoxelMapSetting->var.GetValue<MVariantStruct>().GetVariant<MVariantStruct>("voxelMapSetting");

	settingStruct.SetVariant("f3VoxelOrigin", setting.f3VoxelOrigin);
	settingStruct.SetVariant("fResolution", setting.fResolution);
	settingStruct.SetVariant("fVoxelStep", setting.fVoxelStep);
	m_pVoxelMapSetting->SetDirty();
}
