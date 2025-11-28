/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Material/MMaterialPropertyModifier.h"
#include "Material/MMaterialTemplate.h"
#include "Object/MObject.h"
#include "Resource/MTextureResource.h"
#include "Shader/IShaderProgram.h"
#include "Shader/MShaderMacro.h"
#include "Shader/MShaderParameterSet.h"


namespace morty
{

class MORTY_API MMaterial : public MResource
{
public:
    MORTY_CLASS(MMaterial)

    MMaterial() = default;

    ~MMaterial() override = default;


    template<typename TYPE> void SetValue(const MStringId& strName, const TYPE& value);

    void                         SetTexture(const MStringId& strName, const std::shared_ptr<MResource>& pTexResource);

    [[nodiscard]] MShaderMacro   GetShaderMacro() const;

    [[nodiscard]] std::shared_ptr<MMaterialTemplate> GetTemplate() const;
    void ResetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& newMaterialTemplate);

    [[nodiscard]] MShaderParameterSet*             GetMaterialParameterSet() const;
    [[nodiscard]] const MMaterialPropertyModifier* GetPropertyModifier() const { return &m_propertyModifier; }
    MMaterialPropertyModifier*                     GetPropertyModifier() { return &m_propertyModifier; }

public:
    void                              OnCreated() override;

    void                              OnDelete() override;

    static std::shared_ptr<MMaterial> CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate);

#if MORTY_DEBUG
    [[nodiscard]] const char* GetDebugName() const;
#endif

protected:
    void BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate);

private:
    MResourceRef                         m_materialTemplate;
    std::shared_ptr<MShaderParameterSet> m_materialParameterSet = nullptr;

    MMaterialPropertyModifier            m_propertyModifier;
};

template<typename TYPE> void MMaterial::SetValue(const MStringId& strName, const TYPE& value)
{
    if (!m_propertyModifier.SetValue(strName, value))
    {
        MLogger::GetInstance()->Warning(
                "Failed to set shader property value: {}, material: {}, type: {}",
                strName.c_str(),
                GetDebugName(),
                typeid(TYPE).name()
        );
    }
}

}// namespace morty