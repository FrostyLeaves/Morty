#include "MMaterialTemplateResource.h"
#include "MMaterialResourceData.h"
#include "MMaterialTemplateResourceData.h"
#include "Utility/MFileHelper.h"

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"

MORTY_CLASS_IMPLEMENT(MMaterialTemplateResource, MMaterialTemplate)


bool MMaterialTemplateResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	auto pMaterialData = std::make_unique<MMaterialTemplateResourceData>();

	pMaterialData->eMaterialType = GetMaterialType();
	pMaterialData->eCullMode = GetCullMode();
	pMaterialData->shaderMacro = GetShaderMacro();

	for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
	{
		if (const auto pResource = GetShaderProgram()->GetShaderResource(MEShaderType(nIdx)))
		{
			pMaterialData->vShaders[nIdx] = pResource->GetResourcePath();
		}
	}

	pResourceData = std::move(pMaterialData);
	return true;
}

bool MMaterialTemplateResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
	auto pMaterialData = static_cast<MMaterialTemplateResourceData*>(pResourceData.get());

	SetMaterialType(pMaterialData->eMaterialType);
	SetCullMode(pMaterialData->eCullMode);
	SetCullMode(MECullMode::ECullNone);
	SetShaderMacro(pMaterialData->shaderMacro);

	for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
	{
		if (!pMaterialData->vShaders[nIdx].empty())
		{
			LoadShader(pMaterialData->vShaders[nIdx]);
		}
	}
	
	m_pResourceData = std::move(pResourceData);
	return true;
}

std::shared_ptr<MMaterialTemplate> MMaterialTemplateResource::GetMaterial() const
{
	return DynamicCast<MMaterialTemplate>(GetShared());
}
