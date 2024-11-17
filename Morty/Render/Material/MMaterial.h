/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Resource/MTextureResource.h"

#include "Material/MMaterialTemplate.h"
#include "Shader/MShaderMacro.h"
#include "Shader/MShaderProgram.h"
#include "Shader/MShaderPropertyBlock.h"

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

    [[nodiscard]] MShaderMacro   GetShaderMacro() const { return m_materialTemplate->GetShaderMacro(); }

    [[nodiscard]] MECullMode     GetCullMode() const { return m_materialTemplate->GetCullMode(); }

    [[nodiscard]] MEMaterialType GetMaterialType() const { return m_materialTemplate->GetMaterialType(); }

    [[nodiscard]] bool           GetConservativeRasterizationEnable() const
    {
        return m_materialTemplate->GetConservativeRasterizationEnable();
    }

    [[nodiscard]] Vector2i GetShadingRate() const { return m_materialTemplate->GetShadingRate(); }

    [[nodiscard]] const std::shared_ptr<MShaderProgram>&       GetShaderProgram() const;

    [[nodiscard]] const std::shared_ptr<MShaderPropertyBlock>& GetMaterialPropertyBlock() const;

    [[nodiscard]] const std::shared_ptr<MMaterialTemplate>&    GetMaterialTemplate() const;
    void ResetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& newMaterialTemplate);

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
    std::shared_ptr<MMaterialTemplate>    m_materialTemplate = nullptr;

    std::shared_ptr<MShaderPropertyBlock> m_shaderProperty;
};

template<typename TYPE> void MMaterial::SetValue(const MStringId& strName, const TYPE& value)
{
    if (const auto pProperty = GetMaterialPropertyBlock()) { pProperty->SetValue(strName, value); }
}

}// namespace morty