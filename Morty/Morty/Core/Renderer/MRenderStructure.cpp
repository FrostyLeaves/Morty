#include "MRenderStructure.h"
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include "MDirectX11Renderer.h"
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif

MVertexBuffer::MVertexBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
}

MShaderParam* MShaderBuffer::GetSharedParam(const unsigned int& unCode)
{
	if (unCode >= s_vShaderParams.size())
		return nullptr;

	if (nullptr == s_vShaderParams[unCode])
		return nullptr;

	if (unCode != s_vShaderParams[unCode]->unCode)
		return nullptr;

	//TODO 返回的指针可能被外部长期持有，当容器进行remove操作时，成员可能被析构，导致外部持有的指针失效。有空把容器类型改为存指针的容器。
	return s_vShaderParams[unCode];
}

std::vector<MShaderSampleParam*> MShaderBuffer::s_vSampleParams = std::vector<MShaderSampleParam*>();
std::vector<MShaderTextureParam*> MShaderBuffer::s_vTextureParams = std::vector<MShaderTextureParam*>();
std::vector<MShaderParam*> MShaderBuffer::s_vShaderParams = std::vector<MShaderParam*>();

MShaderBuffer::MShaderBuffer()
{

}

MVertexShaderBuffer::MVertexShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pVertexShader = nullptr;
	m_pInputLayout = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif
}

MPixelShaderBuffer::MPixelShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pPixelShader = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif
}

MInputLayout::MInputLayout()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pInputLayout = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif
}

MTextureBuffer::MTextureBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif

}

// void* MTextureBuffer::GetResourceView()
// {
// #if RENDER_GRAPHICS == MORTY_DIRECTX_11
// 	return m_pShaderResourceView;
// #elif RENDER_GRAPHICS == MORTY_VULKAN
// 	return nullptr;
// #else
// 	return nullptr;
// #endif
// }


MTextureBuffer::~MTextureBuffer()
{

}

MRenderTextureBuffer::MRenderTextureBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
	m_pRenderTargetView = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
}

MDepthTextureBuffer::MDepthTextureBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
	m_pDepthStencilView = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif

}

MShaderParam::MShaderParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
	, var()
	, bDirty(true)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, pBuffer(nullptr)
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
{

}

MShaderTextureParam::MShaderTextureParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
	, pTexture(nullptr)
	, eType(METextureType::ETexture2D)

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
{

}

MShaderSampleParam::MShaderSampleParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
{

}
