/**
 * @File         MRenderPass
 *
 * @Created      2020-07-05 19:33:41
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADER_PARAM_H_
#define _M_MSHADER_PARAM_H_
#include "MGlobal.h"
#include "MVariant.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include <vulkan/vulkan.h>
#endif

class MITexture;

enum MEShaderParamType
{
	EVertex = 1,
	EPixel = 2,
	EBoth = 3,
};

struct MORTY_CLASS MShaderParam
{
	MShaderParam();

	MShaderParam(const MShaderParam& param, const int& unNone);

	MString strName;
	uint32_t unCode;
	uint32_t  eShaderType;

	MVariant var;
	bool bDirty;
	void SetDirty() { bDirty = true; }


#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11Buffer* pBuffer;
	uint32_t unBindPoint;
	uint32_t unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkBufferMemory;

	VkDescriptorSet m_VkDescriptorSet;
	uint32_t unSet;
	uint32_t unBinding;
#endif
};


enum METextureType
{
	ETexture2D = 1,
	ETextureCube = 2,
};

struct MShaderTextureParam
{
	MShaderTextureParam();

	MString strName;
	uint32_t unCode;
	uint32_t  eShaderType;
	MITexture* pTexture;
	METextureType eType;

	bool bDirty;
	void SetDirty() { bDirty = true; }

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	uint32_t unBindPoint;
	uint32_t unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorSet m_VkDescriptorSet;
	uint32_t unSet;
	uint32_t unBinding;
#endif
};

struct MShaderSampleParam
{
	MShaderSampleParam();
	MString strName;
	uint32_t unCode;
	uint32_t  eShaderType;
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	uint32_t unBindPoint;
	uint32_t unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorSet m_VkDescriptorSet;
	uint32_t unSet;
	uint32_t unBinding;
#endif
};

class MShaderSet
{
public:
	MShaderSet();
public:
	std::vector<MShaderParam*> m_vParams;
	std::vector<MShaderTextureParam*> m_vTextures;
	std::vector<MShaderSampleParam*> m_vSamples;

public:
#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
	uint32_t m_unSet;
#endif
};

//Shader
class MShaderBuffer
{
public:
	MShaderBuffer();
	virtual ~MShaderBuffer() {}
	
	union {
		struct {
			MShaderSet m_MaterialSet;
			MShaderSet m_FrameSet;
			MShaderSet m_MeshSet;
		};
		MShaderSet m_vShaderSets[3];
	};

	static MShaderParam* GetSharedParam(const uint32_t& unCode);

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