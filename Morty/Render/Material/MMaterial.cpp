#include "Material/MMaterial.h"
#include "Engine/MEngine.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

void MMaterial::SetTexture(const MStringId& strName, std::shared_ptr<MResource> pResource)
{
    for (size_t i = 0; i < GetMaterialPropertyBlock()->m_textures.size(); ++i)
    {
        const std::shared_ptr<MTextureResourceParam>& pParam =
                std::dynamic_pointer_cast<MTextureResourceParam>(GetMaterialPropertyBlock()->m_textures[i]);
        if (strName == pParam->strName)
        {
            if (std::shared_ptr<MTextureResource> pTexResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
            {
                if (MTexturePtr pTexture = pTexResource->GetTextureTemplate())
                {
                    if (pTexture->GetTextureType() != pParam->eType) { break; }
                }

                pParam->SetTexture(pTexResource);
            }

            break;
        }
    }
}

const std::shared_ptr<MShaderProgram>& MMaterial::GetShaderProgram() const
{
    return m_materialTemplate->GetShaderProgram();
}

const std::shared_ptr<MShaderPropertyBlock>& MMaterial::GetMaterialPropertyBlock() const { return m_shaderProperty; }

const std::shared_ptr<MMaterialTemplate>&    MMaterial::GetMaterialTemplate() const { return m_materialTemplate; }

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

void MMaterial::OnCreated() { Super::OnCreated(); }

void MMaterial::OnDelete()
{
    m_shaderProperty = nullptr;

    Super::OnDelete();
}

void MMaterial::BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate)
{
    m_materialTemplate = pTemplate;

    const auto pProperty = MMaterialTemplate::CreateMaterialPropertyBlock(pTemplate->GetShaderProgram());

    m_shaderProperty = pProperty;
}
