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
#include "MVulkanWrapper.h"
#endif

class MIRenderer;
class MITexture;
class MTextureCube;

enum METextureLayout
{
	ERGBA8 = 0,
	ERGBA16,
	ER32,
};

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
	return (void*)m_VkImageView;
#else
	return nullptr;
#endif
	}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	struct ID3D11Texture2D* m_pTextureBuffer;
	class ID3D11ShaderResourceView* m_pShaderResourceView;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkFormat m_VkTextureFormat;
	VkImage m_VkTextureImage;
	VkImageLayout m_VkImageLayout;
	VkDeviceMemory m_VkTextureImageMemory;
	VkImageView m_VkImageView;
	VkSampler m_VkSampler;
#endif
};

//用于渲染目标的纹理缓存
class MRenderTextureBuffer : public MTextureBuffer
{
public:
	MRenderTextureBuffer();

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



#endif