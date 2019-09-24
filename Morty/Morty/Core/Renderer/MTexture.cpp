#include "MTexture.h"
#include "MIDevice.h"

#include "MLogManager.h"

MTexture::MTexture()
	:m_pImageData(nullptr)
	, m_v2Size(0, 0)
	, m_unImageDataArraySize(0)
	, m_pTextureBuffer(nullptr)
{

}

MTexture::~MTexture()
{

}

void MTexture::SetSize(const Vector2& v2Size)
{
	if (m_unImageDataArraySize < v2Size.x * v2Size.y * 4)
	{
		if (nullptr != m_pImageData)
		{
			delete[] m_pImageData;
		}

		m_unImageDataArraySize = (unsigned int)v2Size.x * v2Size.y * 4;
		m_pImageData = new unsigned char[m_unImageDataArraySize];
	}

	m_v2Size = v2Size;
}

void MTexture::GenerateBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyTexture(&m_pTextureBuffer);

	pDevice->GenerateTexture(&m_pTextureBuffer, this);
}

void MTexture::DestroyTexture(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyTexture(&m_pTextureBuffer);
}

MTextureCube::MTextureCube()
	: m_v2Size(0, 0)
	, m_pTextureBuffer(nullptr)
{
	memset(m_vTexture, 0, sizeof(MTexture*) * 6);
}

MTextureCube::~MTextureCube()
{
}

void MTextureCube::GenerateBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyTexture(&m_pTextureBuffer);

	pDevice->GenerateTextureCube(&m_pTextureBuffer, m_vTexture, false);
}

void MTextureCube::DestroyTexture(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyTexture(&m_pTextureBuffer);
}

void MTextureCube::SetTextures(MTexture* vTexture[6])
{
	m_v2Size = vTexture[0]->GetSize();

	for (int i = 0; i < 6; ++i)
	{
		if (m_v2Size != vTexture[i]->GetSize())
		{
			MLogManager::GetInstance()->Error("Can`t Use Different Size Textures.");
			return;
		}
		m_vTexture[i] = vTexture[i];
	}
}

void MTextureCube::SetTexture(MTexture* pTexture, const MECubeFace& eFace)
{
	m_vTexture[eFace] = pTexture;
}
