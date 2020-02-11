#include "MTextureResource.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "ximage.h"

MTextureResource::MTextureResource()
{
	m_pTexture = new MTexture();
}

MTextureResource::~MTextureResource()
{
	m_pTexture->DestroyTexture(m_pEngine->GetDevice());
	delete m_pTexture;
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

// 	for (int x = 0; x < unWidth; ++x)
// 	{
// 		for (int y = 0; y < unHeight; ++y)
// 		{
// 			RGBQUAD color = image.GetPixelColor(unWidth - 1 - x, unHeight - 1 - y, true);
// 			pData[(y * unWidth + x) * 4 + 0] = color.rgbRed;
// 			pData[(y * unWidth + x) * 4 + 1] = color.rgbGreen;
// 			pData[(y * unWidth + x) * 4 + 2] = color.rgbBlue;
// 			pData[(y * unWidth + x) * 4 + 3] = color.rgbReserved;
// 		}
// 	}

	if (!image.AlphaIsValid())
	{
		image.AlphaCreate();
		image.AlphaSet(255);
	}

	int dataSize = unWidth * unHeight * 4;
	unsigned char* tempData = nullptr;
 	bool result = image.Encode2RGBA(tempData, dataSize, true);

	if (result)
	{
		memcpy(pData, tempData, dataSize);
		return true;
	}

	return true;
}
