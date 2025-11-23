#include "MMaterialResource.h"
#include "MMaterialResourceData.h"
#include "Utility/MFileHelper.h"
#include "MMaterialTemplate_generated.h"

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Shader/MShaderProgram.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterialResource, MMaterial)


bool MMaterialResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    auto pMaterialData = std::make_unique<MMaterialResourceData>();

    if (const auto pMaterialProperty = GetMaterialPropertyBlock())
    {
        for (const auto& param: pMaterialProperty->GetConstantParams())
        {
            MMaterialResourceData::Property prop;
            prop.name  = param->strName.ToString();
            prop.value = MVariant::Clone(param->var);
            pMaterialData->vProperty.push_back(prop);
        }

        for (const auto& texture: pMaterialProperty->GetTextureParams())
        {
            if (auto pTextureResourceParam = dynamic_cast<MTextureResourceParam*>(texture.get()))
            {
                if (auto pResource = pTextureResourceParam->GetTextureResource())
                {
                    MMaterialResourceData::Texture tex;
                    tex.name  = texture->strName.ToString();
                    tex.value = pResource->GetResourcePath();
                    pMaterialData->vTextures.push_back(tex);
                }
            }
        }
    }

    if (auto pTemplate = GetTemplate()) { pMaterialData->strTemplateResource = pTemplate->GetResourcePath(); }

    pResourceData = std::move(pMaterialData);
    return true;
}

bool MMaterialResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto             pMaterialData = static_cast<MMaterialResourceData*>(pResourceData.get());

    const auto       pMaterialTemplate = pResourceSystem->LoadResource(pMaterialData->strTemplateResource);
    BindTemplate(MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate));

    const size_t nPropertyNum = pMaterialData->vProperty.size();
    for (size_t nIdx = 0; nIdx < nPropertyNum; ++nIdx)
    {
        const auto      fbProperty = pMaterialData->vProperty[nIdx];
        const MStringId strPropertyName(fbProperty.name.c_str());

        if (auto pConstantParam = GetMaterialPropertyBlock()->FindConstantParam(strPropertyName))
        {
            pConstantParam->var = MVariant::Clone(fbProperty.value);
            pConstantParam->SetDirty();
        }
    }

    const size_t nTextureNum = pMaterialData->vTextures.size();
    for (size_t nIdx = 0; nIdx < nTextureNum; ++nIdx)
    {
        const auto fbTexture        = pMaterialData->vTextures[nIdx];
        const auto pTextureResource = pResourceSystem->LoadResource(fbTexture.value, true);
        SetTexture(MStringId(fbTexture.name.c_str()), pTextureResource);
    }

    m_resourceData = std::move(pResourceData);
    return true;
}

std::shared_ptr<MMaterial> MMaterialResource::GetMaterial() const { return DynamicCast<MMaterial>(GetShared()); }

std::shared_ptr<MMaterialResource> MMaterialResource::CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate
)
{
    if (const auto pTemplate = MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate))
    {
        auto pMaterial = std::make_shared<MMaterialResource>();
        pMaterial->BindTemplate(pTemplate);

        return pMaterial;
    }

    return nullptr;
}
