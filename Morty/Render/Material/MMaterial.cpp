#include "Material/MMaterial.h"
#include "Engine/MEngine.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

void MMaterial::SetTexture(const MStringId& strName, const std::shared_ptr<MResource>& pResource)
{
    if(auto textureResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
    {
        GetMaterialPropertyBlock()->SetTexture(strName, textureResource->GetTextureTemplate());
    }
}

const std::shared_ptr<MMaterialTemplate>&    MMaterial::GetTemplate() const { return m_materialTemplate; }

void MMaterial::ResetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& newMaterialTemplate)
{
    BindTemplate(newMaterialTemplate);
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

void MMaterial::OnCreated() { Super::OnCreated(); }

void MMaterial::OnDelete()
{
    Super::OnDelete();
}

void MMaterial::BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate)
{
    if(m_materialTemplate == pTemplate)
        return;
    
    m_materialTemplate = pTemplate;

    if (m_materialTemplate)
    {
        m_materialPropertyBlock = m_materialTemplate->CreatePropertyBlock(MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    }
    else
    {
        m_materialPropertyBlock = nullptr;
    }
    
}


MShaderPropertyBlock* MMaterial::GetMaterialPropertyBlock() const
{
    return m_materialPropertyBlock.get();
}