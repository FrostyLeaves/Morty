#include "MTextureRenderTarget.h"
#include "MIDevice.h"
#include "MRenderStructure.h"
#include "MTexture.h"

MTextureRenderTarget::MTextureRenderTarget()
	: MIRenderTarget()
	, m_pDevice(nullptr)
	, m_pBackTexture(new MRenderTargetTexture())
	, m_pDepthTexture(new MRenderDepthTexture())
	, m_eRenderTargetType(ERenderNone)
	, m_fWidth(0)
	, m_fHeight(0)
{

}

MTextureRenderTarget::~MTextureRenderTarget()
{
	Release(m_pDevice);
}

MTextureRenderTarget* MTextureRenderTarget::CreateForTexture(MIDevice* pDevice, const unsigned int& eRenderTargetType, const unsigned int& unWidth, const unsigned int& unHeight)
{
	MTextureRenderTarget* pRenderTarget = new MTextureRenderTarget();
	pRenderTarget->m_pDevice = pDevice;
	pRenderTarget->m_eRenderTargetType = eRenderTargetType;
	pRenderTarget->OnResize(unWidth, unHeight);

	return pRenderTarget;
}

void MTextureRenderTarget::OnResize(const unsigned int& nWidth, const unsigned int& nHeight)
{
	if (m_fWidth == nWidth && m_fHeight == nHeight)
		return;

	m_fWidth = nWidth;
	m_fHeight = nHeight;

	if (m_pBackTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderBack))
		m_pBackTexture->SetSize(Vector2(nWidth, nHeight));
	if (m_pDepthTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderDepth))
		m_pDepthTexture->SetSize(Vector2(nWidth, nHeight));

	m_pDevice->DestroyRenderTarget(this);

	m_pDevice->GenerateRenderTarget(this, nWidth, nHeight);
}

void MTextureRenderTarget::Release(MIDevice* pDevice)
{
	pDevice->DestroyRenderTarget(this);

	if (m_pBackTexture)
	{
		m_pBackTexture->DestroyTexture(pDevice);
		delete m_pBackTexture;
		m_pBackTexture = nullptr;
	}

	if (m_pDepthTexture)
	{
		m_pDepthTexture->DestroyTexture(pDevice);
		delete m_pDepthTexture;
		m_pDepthTexture = nullptr;
	}

//	MIRenderTarget::Release(pDevice);
}
