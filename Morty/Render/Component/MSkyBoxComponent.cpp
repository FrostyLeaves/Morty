#include "MSkyBoxComponent.h"

MORTY_CLASS_IMPLEMENT(MSkyBoxComponent, MComponent)

#include "MTextureResource.h"

MSkyBoxComponent::MSkyBoxComponent()
	: MComponent()
	, m_Texture()
{

}

MSkyBoxComponent::~MSkyBoxComponent()
{

}

void MSkyBoxComponent::LoadSkyBoxResource(std::shared_ptr<MResource> pTexture)
{
	if (!pTexture)
		return;

	if (std::shared_ptr<MTextureResource> pTextureResource = MTypeClass::DynamicCast<MTextureResource>(pTexture))
	{
		m_Texture.SetResource(pTexture);
	}
}

std::shared_ptr<MResource> MSkyBoxComponent::GetSkyBoxResource()
{
	return m_Texture.GetResource();
}

void MSkyBoxComponent::LoadDiffuseEnvResource(std::shared_ptr<MResource> pTexture)
{
	if (!pTexture)
		return;

	if (std::shared_ptr<MTextureResource> pTextureResource = MTypeClass::DynamicCast<MTextureResource>(pTexture))
	{
		m_DiffuseEnvTexture.SetResource(pTexture);
	}
}

void MSkyBoxComponent::LoadSpecularEnvResource(std::shared_ptr<MResource> pTexture)
{
	if (!pTexture)
		return;

	if (std::shared_ptr<MTextureResource> pTextureResource = MTypeClass::DynamicCast<MTextureResource>(pTexture))
	{
		m_SpecularEnvTexture.SetResource(pTexture);
	}
}

std::shared_ptr<MResource> MSkyBoxComponent::GetDiffuseEnvResource()
{
	return m_DiffuseEnvTexture.GetResource();
}

MTexture* MSkyBoxComponent::GetDiffuseTexture()
{
	if (std::shared_ptr<MTextureResource> pTexture = m_DiffuseEnvTexture.GetResource<MTextureResource>())
	{
		return pTexture->GetTextureTemplate();
	}

	return nullptr;
}

std::shared_ptr<MResource> MSkyBoxComponent::GetSpecularEnvResource()
{
	return m_SpecularEnvTexture.GetResource();
}

MTexture* MSkyBoxComponent::GetSpecularTexture()
{
	if (std::shared_ptr<MTextureResource> pTexture = m_SpecularEnvTexture.GetResource<MTextureResource>())
	{
		return pTexture->GetTextureTemplate();
	}

	return nullptr;
}
