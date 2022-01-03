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

void MSkyBoxComponent::LoadTexture(MResource* pTexture)
{
	if (!pTexture)
		return;

	if (MTextureResource* pTextureResource = pTexture->DynamicCast<MTextureResource>())
	{
		m_Texture.SetResource(pTexture);
	}
}

MResource* MSkyBoxComponent::GetTexture()
{
	return m_Texture.GetResource();
}