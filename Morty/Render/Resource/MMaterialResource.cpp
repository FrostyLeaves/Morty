#include "MMaterialResource.h"
#include "MMaterialResourceData.h"
#include "Utility/MFileHelper.h"

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"

MORTY_CLASS_IMPLEMENT(MMaterialResource, MMaterial)


bool MMaterialResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	auto pMaterialData = std::make_unique<MMaterialResourceData>();

	pMaterialData->eMaterialType = GetMaterialType();
	pMaterialData->eRasterizerType = GetRasterizerType();
	pMaterialData->shaderMacro = GetShaderMacro();
	if (const auto pVertexResource = GetVertexShaderResource())
	{
		pMaterialData->vertexShader = pVertexResource->GetResourcePath();
	}
	if (const auto pPixelResource = GetPixelShaderResource())
	{
		pMaterialData->pixelShader = pPixelResource->GetResourcePath();
	}

	if (const auto pMaterialProperty = GetMaterialParamSet())
	{
		for (auto param : pMaterialProperty->m_vParams)
		{
			MMaterialResourceData::Property prop;
			prop.name = param->strName;
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
					tex.name = texture->strName;
					tex.value = pResource->GetResourcePath();
					pMaterialData->vTextures.push_back(tex);
				}
			}
		}
	}



	pResourceData = std::move(pMaterialData);
	return true;
}

bool MMaterialResource::Load(std::unique_ptr<MResourceData>& pResourceData)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	auto pMaterialData = static_cast<MMaterialResourceData*>(pResourceData.get());

	SetMaterialType(pMaterialData->eMaterialType);
	SetRasterizerType(pMaterialData->eRasterizerType);

	SetShaderMacro(pMaterialData->shaderMacro);
	LoadVertexShader(pMaterialData->vertexShader);
	LoadPixelShader(pMaterialData->pixelShader);
	
	const size_t nPropertyNum = pMaterialData->vProperty.size();
	for (size_t nIdx = 0; nIdx < nPropertyNum; ++nIdx)
	{
		const auto fbProperty = pMaterialData->vProperty[nIdx];
		const MString& strPropertyName = fbProperty.name;

		if (std::shared_ptr<MShaderConstantParam> pConstantParam = GetMaterialParamSet()->FindConstantParam(strPropertyName))
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
		SetTexture(fbTexture.name, pTextureResource);
	}

	m_pResourceData = std::move(pResourceData);
	return true;
}

std::shared_ptr<MMaterial> MMaterialResource::GetMaterial() const
{
	return DynamicCast<MMaterial>(GetShared());
}
