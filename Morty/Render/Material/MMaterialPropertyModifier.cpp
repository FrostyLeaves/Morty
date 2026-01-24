#include "MMaterialPropertyModifier.h"

using namespace morty;

void MMaterialPropertyModifier::BindPropertyBlock(MShaderParameterSet* materialParamSet, const MShaderPropertyBlock& propertyBlock)
{
    if (materialParamSet == nullptr) { return; }

    for (const auto& [name, property]: propertyBlock.GetProperties())
    {
        MShaderUniformParam* param   = nullptr;
        auto                 variant = materialParamSet->FindValue(property.displayName, &param);
        if (variant.IsValid())
        {
            if (m_modifiedParams.find(property.displayName) != m_modifiedParams.end() && m_modifiedParams[property.displayName].value.GetType() == variant.GetType())
            {
                variant.SetValue(m_modifiedParams[property.displayName].value);
                m_modifiedParams[property.displayName].value = variant;
                m_modifiedParams[property.displayName].param = param;
            }
            else { m_modifiedParams[property.displayName] = {.value = variant, .param = param}; }
        }
    }

    for (const auto& [name, resource]: propertyBlock.GetResources())
    {
        MShaderTextureParam* param = materialParamSet->FindTextureParam(resource.displayName);
        if (param)
        {
            if (m_modifiedResources.find(resource.displayName) != m_modifiedResources.end())
            {
                param->SetTexture(m_modifiedResources[resource.displayName].param->GetTexture());
                m_modifiedResources[resource.displayName].param = param;
            }
            else { m_modifiedResources[resource.displayName] = {.param = param}; }
        }
    }

    for (size_t idx = 0; idx < propertyBlock.GetInstancingProperties().size(); ++idx)
    {
        const auto& property = propertyBlock.GetInstancingProperties()[idx];
        auto*       param    = materialParamSet->FindStorageParam(property.name);
        if (param)
        {
            auto newVariant = MVariant::Clone(param->var);
            for (const auto& param: property.properties)
            {
                if (m_modifiedInstanceParams.find(param.displayName) != m_modifiedInstanceParams.end())
                {
                    newVariant.GetValue<MVariantStruct>().SetVariant(param.displayName, m_modifiedInstanceParams[param.displayName]);
                }

                m_modifiedInstanceParams[param.displayName].value = newVariant.GetValue<MVariantStruct>().GetVariant<MVariant>(param.displayName);
            }
            m_modifiedInstanceValue[idx] = std::move(newVariant);
        }
    }
}

bool MMaterialPropertyModifier::SetTexture(const MStringId& strName, const MTexturePtr& resource)
{
    if (m_modifiedResources.find(strName) != m_modifiedResources.end())
    {
        m_modifiedResources[strName].param->SetTexture(resource);
        return true;
    }

    return false;
}

bool MMaterialPropertyModifier::SetTexture(const MStringId& strName, const std::shared_ptr<MTextureResource>& resource)
{
    if (m_modifiedResources.find(strName) != m_modifiedResources.end())
    {
        if (auto resourceParam = dynamic_cast<MTextureResourceParam*>(m_modifiedResources[strName].param))
        {
            resourceParam->SetTexture(resource);
            return true;
        }
    }

    return false;
}

MTexturePtr MMaterialPropertyModifier::GetTexture(const MStringId& strName) const
{
    auto it = m_modifiedResources.find(strName);
    if (it != m_modifiedResources.end() && it->second.param) { return it->second.param->GetTexture(); }

    return nullptr;
}