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

std::wstring s2ws(const std::string &s)
{
	size_t i;
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const char* _source = s.c_str();
	size_t _dsize = s.size() + 1;
	wchar_t* _dest = new wchar_t[_dsize];
	wmemset(_dest, 0x0, _dsize);
	mbstowcs_s(&i, _dest, _dsize, _source, _dsize);
	std::wstring result = _dest;
	delete[] _dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

bool MTextureResource::Load(const MString& strResourcePath)
{
	CxImage image;
	if (false == image.Load(strResourcePath.c_str()))
		return false;
	
	unsigned int unWidth = image.GetWidth();
	unsigned int unHeight = image.GetHeight();

	m_pTexture->SetSize(Vector2(unWidth, unHeight));

	unsigned char* pData = m_pTexture->GetImageData();

	for (int x = 0; x < unWidth; ++x)
	{
		for (int y = 0; y < unHeight; ++y)
		{
			RGBQUAD color = image.GetPixelColor(x, y, true);
			pData[(y * unWidth + x) * 4 + 0] = color.rgbRed;
			pData[(y * unWidth + x) * 4 + 1] = color.rgbGreen;
			pData[(y * unWidth + x) * 4 + 2] = color.rgbBlue;
			pData[(y * unWidth + x) * 4 + 3] = color.rgbReserved;
		}
	}

// 	int dataSize = unWidth * unHeight * 4;
//  	image.Encode2RGBA(pData, dataSize, false);


	return true;
}
