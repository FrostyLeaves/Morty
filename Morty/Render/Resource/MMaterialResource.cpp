#include "MMaterialResource.h"
#include "MMaterialResourceData.h"
#include "MMaterialTemplate_generated.h"
#include "Utility/MFileHelper.h"

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterialResource, MMaterial)


bool MMaterialResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	auto pMaterialData = std::make_unique<MMaterialResourceData>();

	if (const auto pMaterialProperty = GetMaterialPropertyBlock())
	{
		for (auto param : pMaterialProperty->m_vParams)
		{
			MMaterialResourceData::Property prop;
			prop.name = param->strName.ToString();
			prop.value = MVariant::Clone(param->var);
			pMaterialData->vProperty.push_back(prop);
		}

		for (auto texture : pMaterialProperty->m_vTextures)
		{
			if (auto pTextureResourceParam = std::dynamic_pointer_cast<MTextureResourceParam>(texture))
			{
				if (auto pResource = pTextureResourceParam->GetTextureResource())
				{
					MMaterialResourceData::Texture tex;
					tex.name = texture->strName.ToString();
					tex.value = pResource->GetResourcePath();
					pMaterialData->vTextures.push_back(tex);
				}
			}
		}
	}

	pMaterialData->strTemplateResource = GetMaterialTemplate()->GetResourcePath();

	pResourceData = std::move(pMaterialData);
	return true;
}

bool MMaterialResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pMaterialData = static_cast<MMaterialResourceData*>(pResourceData.get());

	const auto pMaterialTemplate = pResourceSystem->LoadResource(pMaterialData->strTemplateResource);
	BindTemplate(MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate));
	
	const size_t nPropertyNum = pMaterialData->vProperty.size();
	for (size_t nIdx = 0; nIdx < nPropertyNum; ++nIdx)
	{
		const auto fbProperty = pMaterialData->vProperty[nIdx];
		const MStringId strPropertyName(fbProperty.name.c_str());

		if (std::shared_ptr<MShaderConstantParam> pConstantParam = GetMaterialPropertyBlock()->FindConstantParam(strPropertyName))
		{
			pConstantParam->var = MVariant::Clone(fbProperty.value);
			pConstantParam->SetDirty();
		}
	}

	const size_t nTextureNum = pMaterialData->vTextures.size();
	for (size_t nIdx = 0; nIdx < nTextureNum; ++nIdx)
	{
		const auto fbTexture = pMaterialData->vTextures[nIdx];
		const auto pTextureResource = pResourceSystem->LoadResource(fbTexture.value, true);
		SetTexture(MStringId(fbTexture.name.c_str()), pTextureResource);
	}

	m_pResourceData = std::move(pResourceData);
	return true;
}

std::shared_ptr<MMaterial> MMaterialResource::GetMaterial() const
{
	return DynamicCast<MMaterial>(GetShared());
}

std::shared_ptr<MMaterialResource> MMaterialResource::CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate)
{
	if (const auto pTemplate = MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate))
	{
		auto pMaterial = std::make_shared<MMaterialResource>();
		pMaterial->BindTemplate(pTemplate);

		return pMaterial;
	}

	return nullptr;
}
