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
    void                         SetTexture(const MStringId& name, const MResourcePtr& texture);

    [[nodiscard]] MEMaterialType GetMaterialType() const { return GetMaterialTemplate()->GetMaterialType(); }

    [[nodiscard]] const std::shared_ptr<MShaderProgram>&    GetShaderProgram() const;

    [[nodiscard]] const std::shared_ptr<MMaterialTemplate>& GetMaterialTemplate() const;
    void ResetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& newMaterialTemplate);

public:
    void OnCreated() override;

    void OnDelete() override;

#if MORTY_DEBUG
    [[nodiscard]] const char* GetDebugName() const;
#endif

protected:
    void BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate);

private:
    std::shared_ptr<MMaterialTemplate> m_materialTemplate = nullptr;

    MStruct                            m_variant;
};

template<typename TYPE> void MMaterial::SetValue(const MStringId& strName, const TYPE& value)
{
    //TODO Material Refactor

    MORTY_UNUSED(strName);
    MORTY_UNUSED(value);
}

}// namespace morty