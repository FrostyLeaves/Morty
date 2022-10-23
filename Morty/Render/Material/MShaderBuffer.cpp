#include "MShaderBuffer.h"


MShaderBuffer::MShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkShaderModule = VK_NULL_HANDLE;
#endif

    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_SKELETON] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_SKELETON);
}

MVertexShaderBuffer::MVertexShaderBuffer()
{
}

MPixelShaderBuffer::MPixelShaderBuffer()
{
}

MComputeShaderBuffer::MComputeShaderBuffer()
{
}
