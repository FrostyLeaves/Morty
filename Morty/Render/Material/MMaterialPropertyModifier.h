/**
 * @File         MMaterialPropertyModifier
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Material/MMaterialTemplate.h"
#include "Object/MObject.h"
#include "Shader/IShaderProgram.h"
#include "Shader/MShaderMacro.h"
#include "Shader/MShaderParameterSet.h"


namespace morty
{

class MORTY_API MMaterialPropertyModifier
{

public:
    struct ModifiedParam {
        MVariant             value;
        MShaderUniformParam* param = nullptr;
    };
    struct ModifiedResource {
        MShaderTextureParam* param = nullptr;
    };

    explicit MMaterialPropertyModifier() = default;
    void BindPropertyBlock(MShaderParameterSet* materialParamSet, const MShaderPropertyBlock& propertyBlock);


    template<typename TYPE> bool                  SetValue(const MStringId& strName, const TYPE& value);
    bool                                          SetTexture(const MStringId& strName, const MTexturePtr& resource);

    template<typename TYPE> bool                  GetValue(const MStringId& strName, TYPE& value) const;
    [[nodiscard]] MTexturePtr                     GetTexture(const MStringId& strName) const;

    std::unordered_map<MStringId, ModifiedParam>& GetModifiedParams() { return m_modifiedParams; }
    [[nodiscard]] const std::unordered_map<MStringId, ModifiedParam>& GetModifiedParams() const
    {
        return m_modifiedParams;
    }
    [[nodiscard]] const std::unordered_map<MStringId, ModifiedResource>& GetModifiedResources() const
    {
        return m_modifiedResources;
    }

private:
    std::unordered_map<MStringId, ModifiedParam>    m_modifiedParams;
    std::unordered_map<MStringId, ModifiedResource> m_modifiedResources;
};

template<typename TYPE> inline bool MMaterialPropertyModifier::SetValue(const MStringId& strName, const TYPE& value)
{
    if (m_modifiedParams.find(strName) != m_modifiedParams.end() && m_modifiedParams[strName].value.IsType<TYPE>())
    {
        m_modifiedParams[strName].value.SetValue(value);
        m_modifiedParams[strName].param->SetDirty();
        return true;
    }

    return false;
}

template<>
inline bool MMaterialPropertyModifier::SetValue<MVariant>(const morty::MStringId& strName, const MVariant& value)
{
    if (m_modifiedParams.find(strName) != m_modifiedParams.end() &&
        m_modifiedParams[strName].value.GetType() == value.GetType())
    {
        m_modifiedParams[strName].value.SetValue(value);
        m_modifiedParams[strName].param->SetDirty();
        return true;
    }

    return false;
}

template<typename TYPE> inline bool MMaterialPropertyModifier::GetValue(const MStringId& strName, TYPE& value) const
{
    auto it = m_modifiedParams.find(strName);
    if (it != m_modifiedParams.end() && it->second.value.IsType<TYPE>())
    {
        value = it->second.value.GetValue<TYPE>();
        return true;
    }

    return false;
}

}// namespace morty