#include "MMaterialTemplateResource.h"
#include "MMaterialResourceData.h"
#include "MMaterialTemplateResourceData.h"
#include "Utility/MFileHelper.h"

#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Flatbuffer/MMaterialPass_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterialTemplateResource, MMaterialTemplate)


bool MMaterialTemplateResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    auto pMaterialData = std::make_unique<MMaterialTemplateResourceData>();

    pMaterialData->shaderMacro = GetShaderMacro();

    // Save material passes
    for (const auto& [passName, pass]: GetPasses())
    {
        if (pass)
        {
            auto passCopy                           = std::make_unique<MMaterialPass>(*pass);
            pMaterialData->materialPasses[passName] = std::move(passCopy);
        }
    }

    for (size_t nIdx = 0; nIdx < size_t(MEShaderType::TOTAL_NUM); ++nIdx)
    {
        if (const auto pResource = GetShaderResource()) { pMaterialData->shaderPath = pResource->GetResourcePath(); }
    }

    pResourceData = std::move(pMaterialData);
    return true;
}

bool MMaterialTemplateResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
    auto pMaterialData = static_cast<MMaterialTemplateResourceData*>(pResourceData.get());

    SetShaderMacro(pMaterialData->shaderMacro);
    LoadShader(pMaterialData->shaderPath);

    // Move material passes from resource data
    for (auto& [passName, pass]: pMaterialData->materialPasses)
    {
        m_passes[passName] = std::make_unique<MMaterialPass>(*pass);
    }

    m_resourceData = std::move(pResourceData);
    return true;
}

std::shared_ptr<MMaterialTemplate> MMaterialTemplateResource::GetMaterial() const
{
    return DynamicCast<MMaterialTemplate>(GetShared());
}
