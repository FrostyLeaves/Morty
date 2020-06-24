/**
 * @File         MVertex
 * 
 * @Created      2019-12-7 16:36:40
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERSTRUCTURE_H_
#define _M_MRENDERSTRUCTURE_H_
#include "MGlobal.h"
#include "Vector.h"
#include "MString.h"
#include "MVariant.h"
#include <map>
#include <vector>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include <vulkan/vulkan.h>
#endif

class MIRenderer;
class MITexture;
class MTextureCube;

class MInputLayout
{
public:
	MInputLayout();
	virtual ~MInputLayout(){}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11InputLayout* m_pInputLayout;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
};

//顶点缓存
class MVertexBuffer
{
public:
	MVertexBuffer();
	virtual ~MVertexBuffer(){}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11Buffer* m_pVertexBuffer;
	class ID3D11Buffer* m_pIndexBuffer;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer m_VkVertexBuffer;
	VkDeviceMemory m_VkVertexBufferMemory;
	VkBuffer m_VkIndexBuffer;
	VkDeviceMemory m_VkIndexBufferMemory;
#endif
};

//纹理缓存
class MTextureBuffer
{
public:
	MTextureBuffer();
	virtual ~MTextureBuffer();

	void* GetResourceView() {
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	return m_pShaderResourceView;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	return nullptr;
#else
	return nullptr;
#endif
	}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	struct ID3D11Texture2D* m_pTextureBuffer;
	class ID3D11ShaderResourceView* m_pShaderResourceView;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkImage m_VkTextureImage;
	VkDeviceMemory m_VkTextureImageMemory;
	VkImageView m_VkImageView;
#endif
};

//用于渲染目标的纹理缓存
class MRenderTextureBuffer : public MTextureBuffer
{
public:
	MRenderTextureBuffer();
	virtual ~MRenderTextureBuffer() {}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	struct ID3D11RenderTargetView* m_pRenderTargetView;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
};

//用于渲染深度的纹理缓存
class MDepthTextureBuffer : public MTextureBuffer
{
public:
	MDepthTextureBuffer();
	virtual ~MDepthTextureBuffer() {}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	struct ID3D11DepthStencilView* m_pDepthStencilView;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
};

enum MEShaderParamType
{
	EVertex = 1,
	EPixel = 2,
	EBoth = 3,
};

struct MShaderParam
{
	MShaderParam();
	MShaderParam(const MShaderParam& param);

	MString strName;
	uint32_t unCode;
	uint32_t  eType;

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

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	uint32_t unBindPoint;
	uint32_t unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
};

struct MShaderSampleParam
{
	MShaderSampleParam();
	MString strName;
	uint32_t unCode;
	uint32_t  eType;
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	uint32_t unBindPoint;
	uint32_t unBindCount;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
};

//Shader
class MShaderBuffer
{
public:
	MShaderBuffer();
	virtual ~MShaderBuffer(){}

	std::vector<MShaderSampleParam*> m_vSampleParamsTemplate;
	std::vector<MShaderTextureParam*> m_vTextureParamsTemplate;
	std::vector<MShaderParam*> m_vShaderParamsTemplate;

	static MShaderParam* GetSharedParam(const uint32_t& unCode);

	static std::vector<MShaderSampleParam*> s_vSampleParams;
	static std::vector<MShaderTextureParam*> s_vTextureParams;
	static std::vector<MShaderParam*> s_vShaderParams;


#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkPipelineShaderStageCreateInfo m_VkShaderStageInfo;
#endif
};

class MVertexShaderBuffer : public MShaderBuffer
{
public:
	MVertexShaderBuffer();
	virtual ~MVertexShaderBuffer(){}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11VertexShader* m_pVertexShader;
	class ID3D11InputLayout* m_pInputLayout;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkPipelineLayout m_VkPipelineLayout;
	VkPipelineVertexInputStateCreateInfo m_VkVertexInputStateInfo;
#endif

};

class MPixelShaderBuffer : public MShaderBuffer
{
public:
	MPixelShaderBuffer();
	virtual ~MPixelShaderBuffer(){}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11PixelShader* m_pPixelShader;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkPipelineLayout m_VkPipelineLayout;
#endif

};


#endif