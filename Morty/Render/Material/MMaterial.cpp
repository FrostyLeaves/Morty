#include "Material/MMaterial.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Engine/MEngine.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"


MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

void MMaterial::SetTexture(const MStringId& strName, std::shared_ptr<MResource> pResource)
{
	for (size_t i = 0; i < GetMaterialPropertyBlock()->m_vTextures.size(); ++i)
	{
		const std::shared_ptr<MTextureResourceParam>& pParam = std::dynamic_pointer_cast<MTextureResourceParam>(GetMaterialPropertyBlock()->m_vTextures[i]);
		if (strName == pParam->strName)
		{
			if (std::shared_ptr<MTextureResource> pTexResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
			{
				if (std::shared_ptr<MTexture> pTexture = pTexResource->GetTextureTemplate())
				{
					if (pTexture->GetTextureType() != pParam->eType)
					{
						break;
					}
				}
				
				pParam->SetTexture(pTexResource);
			}

			break;
		}
	} 
}

const std::shared_ptr<MShaderProgram>& MMaterial::GetShaderProgram() const
{
	return m_pMaterialTemplate->GetShaderProgram();
}

const std::shared_ptr<MShaderPropertyBlock>& MMaterial::GetMaterialPropertyBlock() const
{
	return m_pShaderProperty;
}

const std::shared_ptr<MMaterialTemplate>& MMaterial::GetMaterialTemplate() const
{
	return m_pMaterialTemplate;
}

std::shared_ptr<MMaterial> MMaterial::CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate)
{
	if (const auto pTemplate = MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate))
	{
		auto pMaterial = std::make_shared<MMaterial>();
		pMaterial->BindTemplate(pTemplate);

		return pMaterial;
	}

	return nullptr;
}

void MMaterial::OnCreated()
{
	Super::OnCreated();
}

void MMaterial::OnDelete()
{
	m_pShaderProperty = nullptr;
		
	Super::OnDelete();
}

#if MORTY_DEBUG
const char* MMaterial::GetDebugName() const
{
	return GetResourcePath().c_str();
}
#endif

void MMaterial::BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate)
{
	m_pMaterialTemplate = pTemplate;

	const auto pProperty = MMaterialTemplate::CreateMaterialPropertyBlock(pTemplate->GetShaderProgram());

	m_pShaderProperty = pProperty;
}
