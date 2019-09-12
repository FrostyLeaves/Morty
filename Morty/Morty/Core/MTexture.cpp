#include "MTexture.h"
#include "MIRenderer.h"

MTexture::MTexture()
	:m_pImageData(nullptr)
	, m_v2Size(0, 0)
	, m_unImageDataArraySize(0)
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

		m_unImageDataArraySize = v2Size.x * v2Size.y * 4;
		m_pImageData = new unsigned char[m_unImageDataArraySize];
	}

	m_v2Size = v2Size;
}

void MTexture::GenerateBuffer(MIRenderer* pRenderer)
{
	if (m_pTextureBuffer)
		pRenderer->DestroyTexture(&m_pTextureBuffer);

	pRenderer->GenerateTexture(&m_pTextureBuffer, this);
}
