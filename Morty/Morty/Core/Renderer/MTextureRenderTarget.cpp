#include "MTextureRenderTarget.h"
#include "MIDevice.h"

MTextureRenderTarget::MTextureRenderTarget(MIDevice* m_pDevice)
	: MIRenderTarget()
	, m_pDevice(m_pDevice)
	, m_pTargetTexture(nullptr)
{

}

MTextureRenderTarget::~MTextureRenderTarget()
{

}

MTextureRenderTarget* MTextureRenderTarget::CreateForTexture(MIDevice* pDevice, const unsigned int& unWidth, const unsigned int& unHeight)
{
	MTextureRenderTarget* pRenderTarget = new MTextureRenderTarget(pDevice);
	pRenderTarget->OnResize(unWidth, unHeight);

	return pRenderTarget;
}

void MTextureRenderTarget::OnResize(int nWidth, int nHeight)
{
	m_pDevice->DestroyRenderTarget(this);
	//TODO resize texture size and buffer.
	//Update pBackBuffer.
	m_pDevice->GenerateRenderTarget(this, nWidth, nHeight);
}
