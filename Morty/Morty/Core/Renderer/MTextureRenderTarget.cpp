#include "MTextureRenderTarget.h"
#include "MIDevice.h"
#include "MRenderStructure.h"
#include "MTexture.h"
#include "MEngine.h"

#include "MFunction.h"

M_OBJECT_IMPLEMENT(MTextureRenderTarget, MObject)

MTextureRenderTarget::MTextureRenderTarget()
	: MIRenderTarget()
	, m_vBackTexture()
	, m_pDepthTexture(nullptr)
	, m_eRenderTargetType(ERenderNone)
	, m_fWidth(0)
	, m_fHeight(0)
{
}

MTextureRenderTarget::~MTextureRenderTarget()
{
	Release(m_pEngine->GetDevice());
}

MRenderTextureBuffer* MTextureRenderTarget::GetBackBuffer(const uint32_t& unIndex)
{
	if (MRenderTargetTexture* pTexture = GetBackTexture(unIndex))
	{
		return pTexture->GetRenderBuffer();
	}

	return nullptr;
}

MColor MTextureRenderTarget::GetBackClearColor(const uint32_t& unIndex)
{
	return m_vBackClearColor[unIndex];
}

MRenderTargetTexture* MTextureRenderTarget::GetBackTexture(const uint32_t& unIndex)
{
	return m_vBackTexture[unIndex];
}

void MTextureRenderTarget::SetBackTexture(MRenderTargetTexture* pBackTexture, const uint32_t& unIndex, const bool& bClearWhenRender, const MColor& clearColor)
{
	if (m_vBackTexture.size() < unIndex + 1)
	{
		m_vBackTexture.resize(unIndex + 1);
		m_vBackClearColor.resize(unIndex + 1);

		m_RenderPass.m_vBackDesc.resize(unIndex + 1);
	}

	m_vBackTexture[unIndex] = pBackTexture;
	m_vBackClearColor[unIndex] = clearColor;
	m_RenderPass.m_vBackDesc[unIndex].bClearWhenRender = bClearWhenRender;
}

void MTextureRenderTarget::SetDepthTexture(MRenderDepthTexture* pDepthTexture, const bool& bClearWhenRender)
{
	m_pDepthTexture = pDepthTexture;
	m_RenderPass.m_DepthDesc.bClearWhenRender = bClearWhenRender;
}

#if RENDER_GRAPHICS == MORTY_VULKAN
VkFramebuffer MTextureRenderTarget::GetFrameBuffer(const uint32_t& unIndex)
{
	//TODO mutil rendertarget
	return m_vFrameBuffer[unIndex];
}
#endif

// void MTextureRenderTarget::Initialize(const uint32_t& eType, const uint32_t& unWidth, const uint32_t& unHeight)
// {
// 	static std::vector<MERenderTextureType> vDefaultArray{MERenderTextureType::ERGBA8};
// 	Initialize(eType, unWidth, unHeight, vDefaultArray);
// }

void MTextureRenderTarget::ResizeAllTexture(const Vector2& v2Size)
{
	for (uint32_t i = 0; i < m_vBackTexture.size(); ++i)
	{
		if (m_vBackTexture[i])
			m_vBackTexture[i]->SetSize(v2Size);
	}

	if (m_pDepthTexture)
		m_pDepthTexture->SetSize(v2Size);
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

	InitRenderPass();
}

void MTextureRenderTarget::Release(MIDevice* pDevice)
{

}

void MTextureRenderTarget::InitRenderPass()
{
	m_RenderPass.m_vSubpass.push_back(MSubpass());
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
