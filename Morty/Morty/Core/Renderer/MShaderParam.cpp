#include "MShaderParam.h"


MShaderParam* MShaderBuffer::GetSharedParam(const uint32_t& unCode)
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

MShaderBuffer::MShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	m_VkShaderModule = VK_NULL_HANDLE;
#endif
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

MShaderParam::MShaderParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
	, var()
	, bDirty(true)
	, eShaderType(0)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, pBuffer(nullptr)
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_VkBuffer(VK_NULL_HANDLE)
	, m_VkBufferMemory(VK_NULL_HANDLE)
	, unSet(0)
	, unBinding(0)
	, m_VkDescriptorSet(VK_NULL_HANDLE)
#endif
{

}

MShaderParam::MShaderParam(const MShaderParam& param, const int& unNone)
	: strName(param.strName)
	, unCode(param.unCode)
	, var(param.var)
	, bDirty(true)
	, eShaderType(param.eShaderType)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, pBuffer(nullptr)
	, unBindPoint(param.unBindPoint)
	, unBindCount(param.unBindCount)
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_VkBuffer(VK_NULL_HANDLE)
	, m_VkBufferMemory(VK_NULL_HANDLE)
	, m_VkDescriptorSet(param.m_VkDescriptorSet)
	, unSet(param.unSet)
	, unBinding(param.unBinding)
#endif
{

}

MShaderTextureParam::MShaderTextureParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
	, pTexture(nullptr)
	, eType(METextureType::ETexture2D)
	, eShaderType(0)
	, bDirty(true)

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_VkDescriptorSet(VK_NULL_HANDLE)
	, unSet(0)
	, unBinding(0)
#endif
{

}

MShaderSampleParam::MShaderSampleParam()
	: strName()
	, unCode(SHADER_PARAM_CODE_DEFAULT)
	, eShaderType(0)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, unBindPoint(0)
	, unBindCount(0)
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_VkDescriptorSet(VK_NULL_HANDLE)
	, unSet(0)
	, unBinding(0)
#endif
{

}

MShaderSet::MShaderSet()
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	, m_unSet(0)
#endif
{

}
