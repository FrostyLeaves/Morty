#include "MTextureResource.h"
#include "MResourceManager.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "ximage.h"

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
	CxImage image;
	unsigned int imageType = CxImage::GetTypeIdFromName(GetSuffix(strResourcePath).c_str());
	if (false == image.Load(strResourcePath.c_str(), imageType))
		return false;
	
	unsigned int unWidth = image.GetWidth();
	unsigned int unHeight = image.GetHeight();

	m_pTexture->DestroyTexture(m_pEngine->GetDevice());

	m_pTexture->SetSize(Vector2(unWidth, unHeight));

	unsigned char* pData = m_pTexture->GetImageData();

	if (!image.AlphaIsValid())
	{
		image.AlphaCreate();
		image.AlphaSet(255);
	}

	for (int x = 0; x < unWidth; ++x)
	{
		for (int y = 0; y < unHeight; ++y)
		{
			RGBQUAD color = image.GetPixelColor(x, unHeight - 1 - y, true);
			pData[(y * unWidth + x) * 4 + 0] = color.rgbRed;
			pData[(y * unWidth + x) * 4 + 1] = color.rgbGreen;
			pData[(y * unWidth + x) * 4 + 2] = color.rgbBlue;
			pData[(y * unWidth + x) * 4 + 3] = color.rgbReserved;
		}
	}

	m_pTexture->GenerateBuffer(m_pEngine->GetDevice());

	
	return true;
}
