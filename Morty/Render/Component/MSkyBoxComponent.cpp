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

void MSkyBoxComponent::LoadSkyBoxResource(MResource* pTexture)
{
	if (!pTexture)
		return;

	if (MTextureResource* pTextureResource = pTexture->DynamicCast<MTextureResource>())
	{
		m_Texture.SetResource(pTexture);
	}
}

MResource* MSkyBoxComponent::GetSkyBoxResource()
{
	return m_Texture.GetResource();
}

void MSkyBoxComponent::LoadDiffuseEnvResource(MResource* pTexture)
{
	if (!pTexture)
		return;

	if (MTextureResource* pTextureResource = pTexture->DynamicCast<MTextureResource>())
	{
		m_DiffuseEnvTexture.SetResource(pTexture);
	}
}

void MSkyBoxComponent::LoadSpecularEnvResource(MResource* pTexture)
{
	if (!pTexture)
		return;

	if (MTextureResource* pTextureResource = pTexture->DynamicCast<MTextureResource>())
	{
		m_SpecularEnvTexture.SetResource(pTexture);
	}
}

MResource* MSkyBoxComponent::GetDiffuseEnvResource()
{
	return m_DiffuseEnvTexture.GetResource();
}

MTexture* MSkyBoxComponent::GetDiffuseTexture()
{
	if (MTextureResource* pTexture = m_DiffuseEnvTexture.GetResource<MTextureResource>())
	{
		return pTexture->GetTextureTemplate();
	}

	return nullptr;
}

MResource* MSkyBoxComponent::GetSpecularEnvResource()
{
	return m_SpecularEnvTexture.GetResource();
}

MTexture* MSkyBoxComponent::GetSpecularTexture()
{
	if (MTextureResource* pTexture = m_SpecularEnvTexture.GetResource<MTextureResource>())
	{
		return pTexture->GetTextureTemplate();
	}

	return nullptr;
}
