#include "MTextureRenderTarget.h"
#include "MIDevice.h"
#include "MRenderStructure.h"
#include "MTexture.h"
#include "MEngine.h"

#include "MFunction.h"

M_OBJECT_IMPLEMENT(MTextureRenderTarget, MObject)

MTextureRenderTarget::MTextureRenderTarget()
	: MIRenderTarget()
	, m_vBackTexture(nullptr)
	, m_pDepthTexture(new MRenderDepthTexture())
	, m_vBackgroundColor(nullptr)
	, m_eRenderTargetType(ERenderNone)
	, m_fWidth(0)
	, m_fHeight(0)
{
	m_unTargetViewNum = 1;

	for (uint32_t i = 0; i < 8; ++i)
		m_vNeedCleanBeforeRender[i] = true;
}

MTextureRenderTarget::~MTextureRenderTarget()
{
	Release(m_pEngine->GetDevice());
}

void MTextureRenderTarget::SetBackgroundColor(const uint32_t& unTargetIndex, const MColor& color)
{
	M_RETURN_OVER_RANGE(unTargetIndex, 0, m_unTargetViewNum);

	m_vBackgroundColor[unTargetIndex] = color;
}

const MColor& MTextureRenderTarget::GetBackgroundColor(const uint32_t& unTargetIndex) const
{
	return m_vBackgroundColor[unTargetIndex];
}

bool MTextureRenderTarget::GetNeedCleanTargetView(const uint32_t& unTargetIndex) const
{
	M_RETURN_OVER_RANGE(unTargetIndex, 0, 8, (true));

	return m_vNeedCleanBeforeRender[unTargetIndex];
}

void MTextureRenderTarget::Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight)
{
	static std::vector<MERenderTextureType> vDefaultArray{MERenderTextureType::ERGBA8};
	Initialize(eType, unWidth, unHeight, vDefaultArray);
}

void MTextureRenderTarget::Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight, const std::vector<MERenderTextureType>& vTextureTypes)
{
	m_eRenderTargetType = eType;

	if (vTextureTypes.size() > 0)
	{
		m_unTargetViewNum = vTextureTypes.size();
		m_vBackTexture = new MRenderTargetTexture[m_unTargetViewNum];
		m_vBackgroundColor = new MColor[m_unTargetViewNum];

		for (uint32_t i = 0; i < m_unTargetViewNum; ++i)
		{
			m_vBackTexture[i].SetType(vTextureTypes[i]);
		}
	}
	else
	{
		m_unTargetViewNum = 0;
		m_vBackgroundColor = nullptr;
		m_vBackTexture = nullptr;
	}


	OnResize(unWidth, unHeight);
}

void MTextureRenderTarget::OnCreated()
{
	Super::OnCreated();

}

void MTextureRenderTarget::OnResize(const uint32_t& nWidth, const uint32_t& nHeight)
{
	if (m_fWidth == nWidth && m_fHeight == nHeight)
		return;

	m_fWidth = nWidth;
	m_fHeight = nHeight;

	m_pEngine->GetDevice()->DestroyRenderTarget(this);

	Vector2 v2Size(nWidth, nHeight);

	if (m_vBackTexture && (m_eRenderTargetType & METextureRenderTargetType::ERenderBack))
	{
		for (uint32_t i = 0; i < GetTargetViewNum(); ++i)
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
		for (uint32_t i = 0; i < GetTargetViewNum(); ++i)
			m_vBackTexture[i].DestroyTexture(pDevice);

		delete[] m_vBackTexture;
		m_vBackTexture = nullptr;

		delete[] m_vBackgroundColor;
		m_vBackgroundColor = nullptr;
	}

	if (m_pDepthTexture)
	{
		m_pDepthTexture->DestroyTexture(pDevice);
		delete m_pDepthTexture;
		m_pDepthTexture = nullptr;
	}

//	MIRenderTarget::Release(pDevice);
}
