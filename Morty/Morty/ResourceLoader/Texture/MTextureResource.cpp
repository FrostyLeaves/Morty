#include "MTextureResource.h"
#include <tchar.h>

#include "ximage.h"

MTextureResource::MTextureResource()
{
	m_pTexture = new MTexture();
}

MTextureResource::~MTextureResource()
{
	delete m_pTexture;
}

bool MTextureResource::Load(const MString& strResourcePath)
{
	CxImage image;

	const TCHAR* ppp = nullptr;
	image.Load(ppp, 0);
	
	unsigned int unWidth = image.GetWidth();
	unsigned int unHeight = image.GetHeight();

	m_pTexture->SetSize(Vector2(unWidth, unHeight));

	unsigned char* pData = m_pTexture->GetImageData();

	int dataSize = unWidth * unHeight * 4;
	image.Encode2RGBA(pData, dataSize, false);


	return false;
}
