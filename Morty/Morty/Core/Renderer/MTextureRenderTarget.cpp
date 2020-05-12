#include "MTextureRenderTarget.h"
#include "MIDevice.h"
#include "MRenderStructure.h"
#include "MTexture.h"
#include "MEngine.h"

MTypeIdentifierImplement(MTextureRenderTarget, MObject)

MTextureRenderTarget::MTextureRenderTarget()
	: MIRenderTarget()
	, m_vBackTexture(nullptr)
	, m_pDepthTexture(new MRenderDepthTexture())
	, m_eRenderTargetType(ERenderNone)
	, m_fWidth(0)
	, m_fHeight(0)
{
	m_unTargetViewNum = 1;
}

MTextureRenderTarget::~MTextureRenderTarget()
{
	Release(m_pEngine->GetDevice());
}

void MTextureRenderTarget::Initialize(const unsigned int& eType, const unsigned int& unWidth, const unsigned int& unHeight, const unsigned int& unTargetViewNum/* = 1*/)
{
	m_eRenderTargetType = eType;
	m_unTargetViewNum = unTargetViewNum;

	unsigned int unSize = GetTargetViewNum();
	m_vBackTexture = new MRenderTargetTexture[unSize];

	OnResize(unWidth, unHeight);
}

void MTextureRenderTarget::OnCreated()
{
	Super::OnCreated();

}

void MTextureRenderTarget::OnResize(const unsigned int& nWidth, const unsigned int& nHeight)
{
	if (m_fWidth == nWidth && m_fHeight == nHeight)
		return;

	m_fWidth = nWidth;
	m_fHeight = nHeight;

	m_pEngine->GetDevice()->DestroyRenderTarget(this);

	Vector2 v2Size(nWidth, nHeight);

	if (m_vBackTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderBack))
	{
		for (unsigned int i = 0; i < GetTargetViewNum(); ++i)
			m_vBackTexture[i].SetSize(v2Size);
	}

	if (m_pDepthTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderDepth))
		m_pDepthTexture->SetSize(v2Size);


	if (nWidth == 0 || nHeight == 0)
		return;

	m_pEngine->GetDevice()->GenerateRenderTarget(this, nWidth, nHeight);
}

void MTextureRenderTarget::Release(MIDevice* pDevice)
{
	pDevice->DestroyRenderTarget(this);

	if (m_vBackTexture)
	{
		for (unsigned int i = 0; i < GetTargetViewNum(); ++i)
			m_vBackTexture[i].DestroyTexture(pDevice);

		delete[] m_vBackTexture;
		m_vBackTexture = nullptr;
	}

	if (m_pDepthTexture)
	{
		m_pDepthTexture->DestroyTexture(pDevice);
		delete m_pDepthTexture;
		m_pDepthTexture = nullptr;
	}

//	MIRenderTarget::Release(pDevice);
}
