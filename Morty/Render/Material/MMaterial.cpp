#include "Material/MMaterial.h"
#include "Engine/MEngine.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

void MMaterial::SetTexture(const MStringId& name, const std::shared_ptr<MResource>& pResource)
{
    if (auto textureResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
    {
        m_propertyModifier.SetTexture(name, textureResource->GetTextureTemplate());
    }
}

MShaderMacro MMaterial::GetShaderMacro() const
{
    auto temp = m_materialTemplate.GetResource<MMaterialTemplate>();
    if (temp) { return temp->GetShaderMacro(); }
    return {};
}

std::shared_ptr<MMaterialTemplate> MMaterial::GetTemplate() const
{
    auto temp = m_materialTemplate.GetResource<MMaterialTemplate>();
    return temp;
}

void MMaterial::ResetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& newMaterialTemplate)
{
    BindTemplate(newMaterialTemplate);
}

std::shared_ptr<MMaterial> MMaterial::CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate)
{
    if (const auto pTemplate = MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate))
    {
        auto material = std::make_shared<MMaterial>();
        material->BindTemplate(pTemplate);

        return material;
    }

    return nullptr;
}

const char* MMaterial::GetDebugName() const { return GetResourcePath().c_str(); }

void        MMaterial::OnCreated() { Super::OnCreated(); }

void        MMaterial::OnDelete() { Super::OnDelete(); }

void        MMaterial::BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate)
{
    if (m_materialTemplate.GetResource<MMaterialTemplate>() == pTemplate) return;

    auto reloadFunc = [this]() {
        auto temp = m_materialTemplate.GetResource<MMaterialTemplate>();
        if (temp) { m_materialParameterSet = temp->CreateParameterSet(MRenderGlobal::SHADER_PARAM_SET_MATERIAL); }
        else { m_materialParameterSet = nullptr; }

        if (temp && temp->GetDefaultPass() && temp->GetDefaultPass()->GetShaderProgram())
        {
            m_propertyModifier.BindPropertyBlock(
                    m_materialParameterSet.get(),
                    temp->GetDefaultPass()->GetShaderProgram()->GetPropertyBlock()
            );
        }
        else { m_propertyModifier.BindPropertyBlock(nullptr, MShaderPropertyBlock{}); }

        return true;
    };

    m_materialTemplate.SetResource(pTemplate);
    m_materialTemplate.SetPostLoadCallback(reloadFunc);

    reloadFunc();
}


MShaderParameterSet* MMaterial::GetMaterialParameterSet() const { return m_materialParameterSet.get(); }