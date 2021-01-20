/**
 * @File         MShaderBuffer
 * 
 * @Created      2020-07-20 14:41:06
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADERBUFFER_H_
#define _M_MSHADERBUFFER_H_
#include "MGlobal.h"

#include "MShaderParamSet.h"

class MORTY_API MShaderBuffer
{
public:
    MShaderBuffer();
	virtual ~MShaderBuffer() {}

//			MShaderParamSet m_MaterialSet;
//			MShaderParamSet m_FrameSet;
//			MShaderParamSet m_MeshSet;
//			MShaderParamSet m_SkeletonSet;
    MShaderParamSet m_vShaderSets[M_VALID_SHADER_SET_NUM];
	

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkShaderModule m_VkShaderModule;
	VkPipelineShaderStageCreateInfo m_VkShaderStageInfo;
#endif
};


class MVertexShaderBuffer : public MShaderBuffer
{
public:
	MVertexShaderBuffer();
	virtual ~MVertexShaderBuffer() {}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11VertexShader* m_pVertexShader;
	class ID3D11InputLayout* m_pInputLayout;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	std::vector<VkVertexInputAttributeDescription> m_vAttributeDescs;
	std::vector< VkVertexInputBindingDescription> m_vBindingDescs;
#endif

};

class MPixelShaderBuffer : public MShaderBuffer
{
public:
	MPixelShaderBuffer();
	virtual ~MPixelShaderBuffer() {}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11PixelShader* m_pPixelShader;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif

};


#endif
