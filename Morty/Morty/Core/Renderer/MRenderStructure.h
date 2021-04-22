/**
 * @File         MVertex
 * 
 * @Created      2019-12-7 16:36:40
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERSTRUCTURE_H_
#define _M_MRENDERSTRUCTURE_H_
#include "MGlobal.h"
#include "Vector.h"
#include "MString.h"
#include "MVariant.h"

#include <map>
#include <stack>
#include <vector>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"
#endif

class MIDevice;
class MITexture;
class MMaterial;
class MIRenderer;
class MRenderPass;
class MTextureCube;
struct MMaterialPipelineLayoutData;

enum class MEShaderType
{
	ENone = 0,
	EVertex = 1,
	EPixel = 2
};

enum class METextureLayout
{
	ERGBA8 = 0,
	ERGBA16,
	ER32,
	EDepth,
};

enum class METextureUsage
{
	EUnknow = 0,
	ERenderBack,
	ERenderDepth
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

	uint32_t m_unMipmaps;
	uint32_t m_unWidth;
	uint32_t m_unHeight;
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


struct MORTY_API MViewportInfo
{
	MViewportInfo();
	MViewportInfo(const float& _x, const float& _y, const float& _width, const float& _height);

	float x;
	float y;
	float width;
	float height;
	float minz;
	float maxz;
};

struct MORTY_API MScissorInfo
{
	MScissorInfo();
	MScissorInfo(const float& _x, const float& _y, const float& _width, const float& _height);

	float x;
	float y;
	float width;
	float height;
};

struct MORTY_API MRenderPassStage
{
	MRenderPassStage();
	MRenderPassStage(MRenderPass* p, const uint32_t& n);

	MRenderPass* pRenderPass;
	uint32_t nSubpassIdx;
};

class MORTY_API MRenderCommand
{
public:
	MRenderCommand();


public:

	uint32_t m_unFrameIdx;

	MMaterial* pUsingMaterial;
	MMaterialPipelineLayoutData* pUsingPipelineLayoutData;

	std::stack<MRenderPassStage> m_vRenderPassStages;

	//渲染完成回调
	std::vector<std::function<void()>> m_aRenderFinishedCallback;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkCommandBuffer m_VkCommandBuffer;
	VkFence m_VkRenderFinishedFence;
	VkSemaphore m_VkRenderFinishedSemaphore;
#endif
};



#endif