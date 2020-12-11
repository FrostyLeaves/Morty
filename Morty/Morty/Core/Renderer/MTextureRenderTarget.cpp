#include "MTextureRenderTarget.h"
#include "MIDevice.h"
#include "MRenderStructure.h"
#include "MTexture.h"
#include "MEngine.h"

#include "MFunction.h"

M_OBJECT_IMPLEMENT(MTextureRenderTarget, MIRenderTarget)

MTextureRenderTarget::MTextureRenderTarget()
	: MIRenderTarget()
	, m_vBufferInfo()
	, m_eRenderTargetType(ERenderNone)
	, m_fWidth(0)
	, m_fHeight(0)
{
}

MTextureRenderTarget::~MTextureRenderTarget()
{
}

MRenderDepthTexture* MTextureRenderTarget::GetCurrDepthTexture()
{
	MIRenderer* pRenderer = GetEngine()->GetRenderer();
	uint32_t unIndex = pRenderer->GetFrameIndex();

	MFrameBuffer& info = m_vBufferInfo[unIndex];

	return info.pDepthTexture;
}

bool MTextureRenderTarget::GetDepthEnable()
{
	return m_vBufferInfo[0].pDepthTexture;
}

void MTextureRenderTarget::Resize(const Vector2& v2Size)
{
	GetEngine()->GetDevice()->DestroyRenderTarget(this);

	Super::Resize(v2Size);
}

std::vector<MIRenderBackTexture*>* MTextureRenderTarget::GetBackTexture(const uint32_t& unIndex)
{
	MFrameBuffer& info = m_vBufferInfo[unIndex];
	return &info.vBackTextures;
}

void MTextureRenderTarget::SetBackTexture(const std::array<MIRenderBackTexture*, M_BUFFER_NUM>& vBackTexture, const uint32_t& unIndex)
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (m_vBufferInfo[i].vBackTextures.size() <= unIndex)
			m_vBufferInfo[i].vBackTextures.resize(unIndex + 1, nullptr);
	}

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_vBufferInfo[i].vBackTextures[unIndex] = vBackTexture[i];
	}
}

void MTextureRenderTarget::SetDepthTexture(const std::array<MRenderDepthTexture*, M_BUFFER_NUM> vDepthTexture)
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_vBufferInfo[i].pDepthTexture = vDepthTexture[i];
	}
}

#if RENDER_GRAPHICS == MORTY_VULKAN
MFrameBuffer* MTextureRenderTarget::GetFrameBuffer(const uint32_t& unIndex)
{
	return &m_vBufferInfo[unIndex];
}
#endif

// void MTextureRenderTarget::Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight)
// {
// 	static std::vector<MERenderTextureType> vDefaultArray{MERenderTextureType::ERGBA8};
// 	Initialize(eType, unWidth, unHeight, vDefaultArray);
// }

void MTextureRenderTarget::ResizeAllTexture(const Vector2& v2Size)
{
	for (MFrameBuffer& info : m_vBufferInfo)
	{
		for (uint32_t i = 0; i < info.vBackTextures.size(); ++i)
		{
			if (info.vBackTextures[i])
			{
				info.vBackTextures[i]->SetSize(v2Size);
			}
		}

		if (info.pDepthTexture)
		{
			info.pDepthTexture->SetSize(v2Size);
		}
	}
}

// void MTextureRenderTarget::Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight, const std::vector<MERenderTextureType>& vTextureTypes)
// {
// 	m_eRenderTargetType = eType;
// 
// 	if (vTextureTypes.size() > 0)
// 	{
// 		m_unTargetViewNum = vTextureTypes.size();
// 		m_vBackTexture = new MRenderTargetTexture[m_unTargetViewNum];
// 
// 		for (uint32_t i = 0; i < m_unTargetViewNum; ++i)
// 		{
// 			m_vBackTexture[i].SetType(vTextureTypes[i]);
// 		}
// 	}
// 	else
// 	{
// 		m_unTargetViewNum = 0;
// 		m_vBackTexture = nullptr;
// 	}
// 
// 
// 	//OnResize(unWidth, unHeight);
// }

void MTextureRenderTarget::OnCreated()
{
	Super::OnCreated();

}

void MTextureRenderTarget::OnDelete()
{
	Super::OnDelete();
}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
std::vector<struct ID3D11RenderTargetView*> MTextureRenderTarget::GetRenderTargetViews()
{
	std::vector<struct ID3D11RenderTargetView*> views(m_vBackTexture.size());
	for (uint32_t i = 0; i < m_vBackTexture.size(); ++i)
	{
		if (m_vBackTexture[i])
		{
			if (MRenderTextureBuffer* pBuffer = m_vBackTexture[i]->GetRenderBuffer())
			{
				views[i] = pBuffer->m_pRenderTargetView;
			}
		}
	}

	return views;
}

struct ID3D11DepthStencilView* MTextureRenderTarget::GetDepthStencilView()
{
	if (m_pDepthTexture)
	{
		if (MDepthTextureBuffer* pBuffer = m_pDepthTexture->GetDepthBuffer())
		{
			return pBuffer->m_pDepthStencilView;
		}
	}

	return nullptr;
}
#endif
