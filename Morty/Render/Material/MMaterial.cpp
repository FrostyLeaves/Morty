#include "Material/MMaterial.h"
#include "Engine/MEngine.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

const std::shared_ptr<MShaderProgram>& MMaterial::GetShaderProgram() const
{
    return m_materialTemplate->GetShaderProgram();
}

//const std::shared_ptr<MShaderPropertyBlock>& MMaterial::GetMaterialPropertyBlock() const { return m_shaderProperty; }

const std::shared_ptr<MMaterialTemplate>& MMaterial::GetMaterialTemplate() const { return m_materialTemplate; }

void MMaterial::ResetMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& newMaterialTemplate)
{
    BindTemplate(newMaterialTemplate);
}

void MMaterial::OnCreated() { Super::OnCreated(); }

void MMaterial::OnDelete() { Super::OnDelete(); }

void MMaterial::BindTemplate(const std::shared_ptr<MMaterialTemplate>& pTemplate)
{
    m_materialTemplate = pTemplate;

    const auto pProperty = MMaterialTemplate::CreateMaterialPropertyBlock(pTemplate->GetShaderProgram());
}

void MMaterial::SetTexture(const MStringId& name, const MResourcePtr& texture)
{
    MORTY_UNUSED(name);
    MORTY_UNUSED(texture);
}
