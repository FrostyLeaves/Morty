#include "MTextureRenderTarget.h"
#include "MIDevice.h"
#include "MRenderStructure.h"
#include "MTexture.h"

MTextureRenderTarget::MTextureRenderTarget(MIDevice* m_pDevice)
	: MIRenderTarget()
	, m_pDevice(m_pDevice)
	, m_pViewport(nullptr)
	, m_pBackTexture(new MTexture())
	, m_pDepthTexture(new MRenderDepthTexture())
	, m_eRenderTargetType(ERenderNone)
{

}

MTextureRenderTarget::~MTextureRenderTarget()
{
	m_pDevice->DestroyRenderTarget(this);

	if (m_pBackTexture)
	{
		delete m_pBackTexture;
		m_pBackTexture = nullptr;
	}

	if (m_pDepthTexture)
	{
		delete m_pDepthTexture;
		m_pDepthTexture = nullptr;
	}

}

MTextureRenderTarget* MTextureRenderTarget::CreateForTexture(MIDevice* pDevice, const unsigned int& eRenderTargetType, const unsigned int& unWidth, const unsigned int& unHeight)
{
	MTextureRenderTarget* pRenderTarget = new MTextureRenderTarget(pDevice);
	pRenderTarget->m_eRenderTargetType = eRenderTargetType;
	pRenderTarget->OnResize(unWidth, unHeight);

	return pRenderTarget;
}

void MTextureRenderTarget::OnResize(const unsigned int& nWidth, const unsigned int& nHeight)
{
	if (m_pBackTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderBack))
		m_pBackTexture->SetSize(Vector2(nWidth, nHeight));
	if (m_pDepthTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderDepth))
		m_pDepthTexture->SetSize(Vector2(nWidth, nHeight));

	m_pDevice->DestroyRenderTarget(this);

	m_pDevice->GenerateRenderTarget(this, nWidth, nHeight);
}
