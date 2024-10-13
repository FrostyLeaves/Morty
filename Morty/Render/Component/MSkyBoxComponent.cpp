#include "Component/MSkyBoxComponent.h"

#include "MRenderNotify.h"
#include "Resource/MTextureResource.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSkyBoxComponent, MComponent)

MSkyBoxComponent::MSkyBoxComponent()
    : MComponent()
    , m_Texture()
{}

MSkyBoxComponent::~MSkyBoxComponent() {}

void MSkyBoxComponent::LoadSkyBoxResource(std::shared_ptr<MResource> pTexture)
{
    if (!pTexture) return;

    std::shared_ptr<MTextureResource> pTextureResource = MTypeClass::DynamicCast<MTextureResource>(pTexture);
    if (!pTextureResource) { return; }

    if (m_Texture.GetResource() == pTexture) { return; }

    m_Texture.SetResource(pTexture);
    SendComponentNotify(MRenderNotify::NOTIFY_SKYBOX_TEX_CHANGED);
}

std::shared_ptr<MResource> MSkyBoxComponent::GetSkyBoxResource() { return m_Texture.GetResource(); }

void                       MSkyBoxComponent::LoadDiffuseEnvResource(std::shared_ptr<MResource> pTexture)
{
    if (!pTexture) return;

    std::shared_ptr<MTextureResource> pTextureResource = MTypeClass::DynamicCast<MTextureResource>(pTexture);
    if (!pTextureResource) { return; }

    if (m_DiffuseEnvTexture.GetResource() == pTexture) { return; }

    m_DiffuseEnvTexture.SetResource(pTexture);
    SendComponentNotify(MRenderNotify::NOTIFY_DIFFUSE_ENV_TEX_CHANGED);
}

void MSkyBoxComponent::LoadSpecularEnvResource(std::shared_ptr<MResource> pTexture)
{
    if (!pTexture) return;

    std::shared_ptr<MTextureResource> pTextureResource = MTypeClass::DynamicCast<MTextureResource>(pTexture);
    if (!pTextureResource) { return; }

    if (m_SpecularEnvTexture.GetResource() == pTexture) { return; }

    m_SpecularEnvTexture.SetResource(pTexture);
    SendComponentNotify(MRenderNotify::NOTIFY_SPECULAR_ENV_TEX_CHANGED);
}

std::shared_ptr<MResource> MSkyBoxComponent::GetDiffuseEnvResource() { return m_DiffuseEnvTexture.GetResource(); }

MTexturePtr                MSkyBoxComponent::GetDiffuseTexture()
{
    if (std::shared_ptr<MTextureResource> pTexture = m_DiffuseEnvTexture.GetResource<MTextureResource>())
    {
        return pTexture->GetTextureTemplate();
    }

    return nullptr;
}

std::shared_ptr<MResource> MSkyBoxComponent::GetSpecularEnvResource() { return m_SpecularEnvTexture.GetResource(); }

MTexturePtr                MSkyBoxComponent::GetSpecularTexture()
{
    if (std::shared_ptr<MTextureResource> pTexture = m_SpecularEnvTexture.GetResource<MTextureResource>())
    {
        return pTexture->GetTextureTemplate();
    }

    return nullptr;
}
