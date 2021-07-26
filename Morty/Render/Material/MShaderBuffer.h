/**
 * @File         MShaderBuffer
 * 
 * @Created      2020-07-20 14:41:06
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADERBUFFER_H_
#define _M_MSHADERBUFFER_H_
#include "MRenderGlobal.h"
#include "MShaderParamSet.h"

class MORTY_API MShaderBuffer
{
public:
    MShaderBuffer();
	virtual ~MShaderBuffer() {}

    MShaderParamSet m_vShaderSets[MGlobal::SHADER_PARAM_SET_NUM];
	
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


#endif
