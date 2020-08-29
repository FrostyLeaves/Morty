#include "MTexture.h"
#include "MIDevice.h"
#include "MRenderStructure.h"

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
	if (nullptr != m_pImageData)
	{
		delete[] m_pImageData;
		m_pImageData = nullptr;
	}
}

void MTexture::SetSize(const Vector2& v2Size)
{
	if (m_unImageDataArraySize < v2Size.x * v2Size.y * 4)
	{
		if (nullptr != m_pImageData)
		{
			delete[] m_pImageData;
		}

		m_unImageDataArraySize = (uint32_t)v2Size.x * v2Size.y * 4;
		m_pImageData = new unsigned char[m_unImageDataArraySize];
		memset(m_pImageData, 255, m_unImageDataArraySize * sizeof(unsigned char));
	}

	m_v2Size = v2Size;
}

void MTexture::FillColor(const MColor& color) 
{
	const uint32_t r = (uint32_t)(color.r * 255.0f);
	const uint32_t g = (uint32_t)(color.g * 255.0f);
	const uint32_t b = (uint32_t)(color.b * 255.0f);
	const uint32_t a = (uint32_t)(color.a * 255.0f);

	if (m_pImageData)
	{
		for (uint32_t i = 0; i < m_unImageDataArraySize; i += 4)
		{
			m_pImageData[i] = r;
			m_pImageData[i + 1] = g;
			m_pImageData[i + 2] = b;
			m_pImageData[i + 3] = a;
		}
	}
}

void MTexture::GenerateBuffer(MIDevice* pDevice, const bool& bMipmap/* = true*/)
{
	if (m_pTextureBuffer)
		pDevice->DestroyTexture(&m_pTextureBuffer);

	pDevice->GenerateTexture(&m_pTextureBuffer, this, bMipmap);

// 	if (nullptr != m_pImageData)
// 	{
// 		delete[] m_pImageData;
// 		m_pImageData = nullptr;
// 		m_unImageDataArraySize = 0;
// 	}
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

void MTextureCube::GenerateBuffer(MIDevice* pDevice, const bool& bMipmap/* = true*/)
{
	if (m_pTextureBuffer)
		pDevice->DestroyTexture(&m_pTextureBuffer);

	pDevice->GenerateTextureCube(&m_pTextureBuffer, m_vTexture, false/*bMipmap*/); //TODO Fix TextureCube Mipmap
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

MRenderBackTexture::MRenderBackTexture()
	: MIRenderBackTexture()
{

}

void MRenderBackTexture::GenerateBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyRenderTextureBuffer(&m_pTextureBuffer);


	pDevice->GenerateRenderTextureBuffer(&m_pTextureBuffer, m_eRenderType, m_v2Size.x, m_v2Size.y);
}

void MRenderBackTexture::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyRenderTextureBuffer(&m_pTextureBuffer);
}

MRenderSwapchainTexture::MRenderSwapchainTexture()
	: MIRenderBackTexture()
{
	m_pTextureBuffer = new MRenderTextureBuffer();
}

MRenderSwapchainTexture::~MRenderSwapchainTexture()
{
	if (m_pTextureBuffer)
	{
		delete m_pTextureBuffer;
		m_pTextureBuffer = nullptr;
	}
}

void MRenderSwapchainTexture::GenerateBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
	{
		pDevice->GenerateRenderTargetView(m_pTextureBuffer);
	}
}

void MRenderSwapchainTexture::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
	{
		pDevice->DestroyRenderTargetView(m_pTextureBuffer);
	}
}

MRenderDepthTexture::MRenderDepthTexture()
	: MIRenderTexture()
	, m_v2Size(0,0)
	, m_pTextureBuffer(nullptr)
{

}

MTextureBuffer* MRenderDepthTexture::GetBuffer()
{
	return m_pTextureBuffer;
}

void MRenderDepthTexture::GenerateBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyDepthTexture(&m_pTextureBuffer);

	pDevice->GenerateDepthTexture(&m_pTextureBuffer, m_v2Size.x, m_v2Size.y);
}

void MRenderDepthTexture::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pTextureBuffer)
		pDevice->DestroyDepthTexture(&m_pTextureBuffer);
}

MIRenderBackTexture::MIRenderBackTexture()
	: MIRenderTexture()
	, m_eRenderType(METextureLayout::ERGBA8)
	, m_v2Size(0, 0)
	, m_pTextureBuffer(nullptr)
{

}
