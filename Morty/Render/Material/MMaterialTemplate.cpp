#include "Material/MMaterialTemplate.h"

#include "Engine/MEngine.h"
#include "MMaterial.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "System/MShaderProgramSystem.h"
#include "Utility/MUtils.h"
#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterialTemplate, MResource)

bool MMaterialTemplate::LoadShader(const std::shared_ptr<MResource>& pResource)
{
    m_shaderResource.SetResource(std::dynamic_pointer_cast<MShaderResource>(pResource));
    SetDirty();

    return true;
}

bool MMaterialTemplate::LoadShader(const MString& strResource)
{
    auto* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
    {
        return LoadShader(pResource);
    }

    return false;
}

MMaterialPass*
MMaterialTemplate::SetPass(const MStringId& passName, const MStringId& vsEntryName, const MStringId& psEntryName)
{
    MMaterialPass* materialPass = nullptr;

    auto           findResult = m_passes.find(passName);
    if (findResult == m_passes.end())
    {
        materialPass = (m_passes[passName] = std::make_unique<MMaterialPass>(this)).get();
    }
    else { materialPass = findResult->second.get(); }

    materialPass->SetEntry(vsEntryName, MEShaderType::EVertex);
    materialPass->SetEntry(psEntryName, MEShaderType::EPixel);

    return materialPass;
}

MMaterialPass* MMaterialTemplate::GetPass(const MStringId& passName) const
{
    auto findResult = m_passes.find(passName);
    if (findResult != m_passes.end()) { return findResult->second.get(); }

    return nullptr;
}

MMaterialPass* MMaterialTemplate::GetDefaultPass() const { return GetPass(MRenderGlobal::DEFAULT_PASS_NAME); }

std::shared_ptr<MShaderParameterSet> MMaterialTemplate::CreateParameterSet(size_t setIdx) const
{
    if (GetDefaultPass() == nullptr) return nullptr;
    auto shaderProgram = GetDefaultPass()->GetShaderProgram();
    if (!shaderProgram) { return nullptr; }

    auto propertyBlocks = shaderProgram->GetShaderParameterSets();
    if (setIdx >= propertyBlocks.size()) { return nullptr; }

    return propertyBlocks[setIdx] ? propertyBlocks[setIdx]->Clone() : nullptr;
}

MHashCode MMaterialTemplate::GetHashCode() const
{
    MHashCode hash;

    MUtils::HashCombine(hash, m_shaderMacro.GetHashCode());
    MUtils::HashCombine(hash, m_shaderResource.GetHashCode());

    return hash;
}

void MMaterialTemplate::OnCreated() { Super::OnCreated(); }

void MMaterialTemplate::OnDelete() { Super::OnDelete(); }
