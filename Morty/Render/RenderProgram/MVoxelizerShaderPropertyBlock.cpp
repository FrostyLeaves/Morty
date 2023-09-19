#include "MVoxelizerShaderPropertyBlock.h"

#include "Render/MBuffer.h"
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
}

void MVoxelizerShaderPropertyBlock::Release(MEngine* pEngine)
{
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


	m_pRWVoxelTableParam->pBuffer = setting.pVoxelTableBuffer;
	m_pRWVoxelTableParam->SetDirty();
}
