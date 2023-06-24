/**
 * @File         MShaderBuffer
 * 
 * @Created      2020-07-20 14:41:06
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADERBUFFER_H_
#define _M_MSHADERBUFFER_H_
#include "Render/MRenderGlobal.h"
#include "Material/MShaderPropertyBlock.h"

class MORTY_API MShaderBuffer
{
public:
    MShaderBuffer();
	virtual ~MShaderBuffer() {}

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_vShaderSets;
	
#if RENDER_GRAPHICS == MORTY_VULKAN
	VkShaderModule m_VkShaderModule;
	VkPipelineShaderStageCreateInfo m_VkShaderStageInfo;
#endif
};


class MVertexShaderBuffer : public MShaderBuffer
{
public:
	MVertexShaderBuffer();
	virtual ~MVertexShaderBuffer() {}

#if RENDER_GRAPHICS == MORTY_VULKAN
	std::vector<VkVertexInputAttributeDescription> m_vAttributeDescs;
	std::vector< VkVertexInputBindingDescription> m_vBindingDescs;
#endif

};

class MPixelShaderBuffer : public MShaderBuffer
{
public:
	MPixelShaderBuffer();
	virtual ~MPixelShaderBuffer() {}

#if RENDER_GRAPHICS == MORTY_VULKAN
#endif

};

class MComputeShaderBuffer : public MShaderBuffer
{
public:
	MComputeShaderBuffer();
	virtual ~MComputeShaderBuffer() {}

#if RENDER_GRAPHICS == MORTY_VULKAN
#endif
};

#endif
