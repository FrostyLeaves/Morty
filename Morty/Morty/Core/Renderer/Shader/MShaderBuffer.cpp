#include "MShaderBuffer.h"


MShaderBuffer::MShaderBuffer()
	: m_MaterialSet()
	, m_FrameSet()
	, m_MeshSet()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	m_VkShaderModule = VK_NULL_HANDLE;
#endif
}

MShaderConstantParam* MShaderBuffer::GetSharedParam(const uint32_t& unCode)
{
	return nullptr;
	/*
	if (unCode >= s_vShaderParams.size())
		return nullptr;

	if (nullptr == s_vShaderParams[unCode])
		return nullptr;

	if (unCode != s_vShaderParams[unCode]->unCode)
		return nullptr;

	//TODO 返回的指针可能被外部长期持有，当容器进行remove操作时，成员可能被析构，导致外部持有的指针失效。有空把容器类型改为存指针的容器。
	return s_vShaderParams[unCode];
*/
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
