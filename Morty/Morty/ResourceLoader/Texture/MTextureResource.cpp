#include "MTextureResource.h"
#include "MResourceManager.h"

#include "MIDevice.h"
#include "MEngine.h"

//#include "ximage.h"
#include "spot.hpp"

M_RESOURCE_IMPLEMENT(MTextureResource, MResource)

MTextureResource::MTextureResource()
	:MResource()
{
	m_unResourceType = MResourceManager::MEResourceType::Texture;
	m_pTexture = new MTexture();
}

MTextureResource::~MTextureResource()
{
	m_pTexture->DestroyTexture(m_pEngine->GetDevice());
	delete m_pTexture;
	m_pTexture = nullptr;
}

void MTextureResource::OnDelete()
{
	MResource::OnDelete();
}

bool MTextureResource::Load(const MString& strResourcePath)
{
	spot::texture image;

	if (!image.load(strResourcePath))
		return false;

	uint32_t unWidth = image.w;
	uint32_t unHeight = image.h;

	m_pTexture->DestroyTexture(m_pEngine->GetDevice());

	m_pTexture->SetSize(Vector2(unWidth, unHeight));

	unsigned char* pData = m_pTexture->GetImageData();

	memcpy(pData, image.data(), image.size());

	for (int x = 0; x < unWidth; ++x)
	{
		for (int y = 0; y < unHeight; ++y)
		{
			auto color = image.at(x, unHeight - 1 - y);
			pData[(y * unWidth + x) * 4 + 0] = color.r;
			pData[(y * unWidth + x) * 4 + 1] = color.g;
			pData[(y * unWidth + x) * 4 + 2] = color.b;
			pData[(y * unWidth + x) * 4 + 3] = color.a;
		}
	}

	m_pTexture->GenerateBuffer(m_pEngine->GetDevice());

	
	return true;
}
