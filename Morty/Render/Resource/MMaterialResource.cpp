#include "MMaterialResource.h"
#include "MMaterialResourceData.h"
#include "Utility/MFileHelper.h"
#include "MMaterialTemplate_generated.h"

#include "Engine/MEngine.h"
#include "Shader/MShaderProgram.h"
#include "System/MResourceSystem.h"


using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterialResource, MMaterial)


bool MMaterialResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    auto pMaterialData = std::make_unique<MMaterialResourceData>();

    if (auto modifier = GetPropertyModifier())
    {
        for (const auto& [name, modifiedParam]: modifier->GetModifiedParams())
        {
            MMaterialResourceData::Property prop;
            prop.name  = name.ToString();
            prop.value = MVariant::Clone(modifiedParam.value);
            pMaterialData->vProperty.push_back(prop);
        }

        for (const auto& [name, modifiedResource]: modifier->GetModifiedResources())
        {
            if (auto textureResourceParam = dynamic_cast<MTextureResourceParam*>(modifiedResource.param))
            {
                if (auto pResource = textureResourceParam->GetTextureResource())
                {
                    MMaterialResourceData::Texture tex;
                    tex.name  = name.ToString();
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

        SetValue(strPropertyName, fbProperty.value);
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

std::shared_ptr<MMaterialResource>
MMaterialResource::CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate)
{
    if (const auto pTemplate = MTypeClass::DynamicCast<MMaterialTemplate>(pMaterialTemplate))
    {
        auto material = std::make_shared<MMaterialResource>();
        material->BindTemplate(pTemplate);

        return material;
    }

    return nullptr;
}
