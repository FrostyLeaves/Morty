/**
 * @File         MShaderBuffer
 * 
 * @Created      2020-07-20 14:41:06
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"
#include "Shader/MShaderPropertyBlock.h"

class MORTY_API MShaderBuffer
{
public:
    MShaderBuffer();
	virtual ~MShaderBuffer() = default;

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_vShaderSets;
	
#if RENDER_GRAPHICS == MORTY_VULKAN
	VkShaderModule m_VkShaderModule = VK_NULL_HANDLE;
	VkPipelineShaderStageCreateInfo m_VkShaderStageInfo;
#endif
};


class MVertexShaderBuffer : public MShaderBuffer
{
#if RENDER_GRAPHICS == MORTY_VULKAN
public:
	std::vector<VkVertexInputAttributeDescription> m_vAttributeDescs;
	std::vector< VkVertexInputBindingDescription> m_vBindingDescs;
#endif

};

class MPixelShaderBuffer : public MShaderBuffer
{

};

class MComputeShaderBuffer : public MShaderBuffer
{

};

class MGeometryShaderBuffer : public MShaderBuffer
{

};
