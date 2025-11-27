#include "MMaterialPropertyModifier.h"

using namespace morty;

void MMaterialPropertyModifier::BindPropertyBlock(
        MShaderParameterSet*        materialParamSet,
        const MShaderPropertyBlock& propertyBlock
)
{
    if (materialParamSet == nullptr) { return; }

    for (const auto& [name, property]: propertyBlock.GetProperties())
    {
        MShaderUniformParam* param   = nullptr;
        auto                 variant = materialParamSet->FindValue(name, &param);
        if (variant.IsValid())
        {
            if (m_modifiedParams.find(name) != m_modifiedParams.end() &&
                m_modifiedParams[name].value.GetType() == variant.GetType())
            {
                variant.SetValue(m_modifiedParams[name].value);
                m_modifiedParams[name].value = variant;
                m_modifiedParams[name].param = param;
            }
            else { m_modifiedParams[name] = {.value = variant, .param = param}; }
        }
    }

    for (const auto& [name, resource]: propertyBlock.GetResources())
    {
        MShaderTextureParam* param = materialParamSet->FindTextureParam(name);
        if (param)
        {
            if (m_modifiedResources.find(name) != m_modifiedResources.end())
            {
                param->SetTexture(m_modifiedResources[name].param->GetTexture());
                m_modifiedResources[name].param = param;
            }
            else { m_modifiedResources[name] = {.param = param}; }
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