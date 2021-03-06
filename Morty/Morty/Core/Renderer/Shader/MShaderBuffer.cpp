#include "MShaderBuffer.h"


MShaderBuffer::MShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	m_VkShaderModule = VK_NULL_HANDLE;
#endif

    m_vShaderSets[MGlobal::SHADER_PARAM_SET_MATERIAL] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_MATERIAL);
    m_vShaderSets[MGlobal::SHADER_PARAM_SET_FRAME] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_FRAME);
    m_vShaderSets[MGlobal::SHADER_PARAM_SET_MESH] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_MESH);
    m_vShaderSets[MGlobal::SHADER_PARAM_SET_SKELETON] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_SKELETON);
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
