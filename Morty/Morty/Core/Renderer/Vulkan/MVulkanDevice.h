/**
 * @File         MVulkanDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVULKANDEVICE_H_
#define _M_MVULKANDEVICE_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MIDevice.h"

#include <vector>
#include "MVulkanWrapper.h"

#ifdef MORTY_WIN
#include "vulkan/vulkan_core.h"
#endif

#include "MVulkanObjectDestructor.h"
#include "MVulkanShaderCompiler.h"
#include "MVulkanPipelineManager.h"
#include "MVulkanBufferPool.h"

#include "MTexture.h"

class MORTY_API MVulkanDevice : public MIDevice
{
public:
    MVulkanDevice();
    virtual ~MVulkanDevice();

public:
	virtual bool Initialize() override;
	virtual void Release() override;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) override;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) override;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) override;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture) override;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap) override;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) override;

	virtual bool GenerateRenderTextureBuffer(MRenderTextureBuffer* ppTextureBuffer, MIRenderTexture* pTexture) override;
	virtual void DestroyRenderTextureBuffer(MRenderTextureBuffer* ppTextureBuffer) override;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro) override;
	virtual void CleanShader(MShaderBuffer** ppShaderBuffer) override;

	virtual bool GenerateShaderParamSet(MShaderParamSet* pParamSet) override;
	virtual void DestroyShaderParamSet(MShaderParamSet* pParamSet) override;

	virtual bool GenerateRenderTarget(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget) override;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) override;

	virtual bool GenerateShaderParamBuffer(MShaderConstantParam* pParam) override;
	virtual void DestroyShaderParamBuffer(MShaderConstantParam* pParam) override;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) override;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) override;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) override;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) override;

	virtual bool GenerateRenderTargetView(MRenderTextureBuffer* pTextureBuffer) override;
	virtual void DestroyRenderTargetView(MRenderTextureBuffer* pTextureBuffer) override;

	virtual bool RegisterMaterial(MMaterial* pMaterial) override;
	virtual bool UnRegisterMaterial(MMaterial* pMaterial) override;


	virtual MRenderCommand* CreateRenderCommand() override;
	virtual void RecoveryRenderCommand(MRenderCommand* pCommand) override;


	VkPhysicalDevice GetPhysicalDevice() { return m_VkPhysicalDevice; }

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height, const uint32_t& unCount);

	void CopyImageBuffer(MTextureBuffer* pSource, MTextureBuffer* pDestination, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void GenerateMipmaps(MTextureBuffer* pBuffer, const uint32_t& unMipLevels, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange, VkCommandBuffer buffer = VK_NULL_HANDLE);

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unMipmap);

	void CreateImage(const uint32_t& unWidth, const uint32_t& unHeight, const uint32_t& unMipmap, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void CreateImageCube(const uint32_t& unWidth, const uint32_t& unHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkCommandBuffer BeginCommands();
	void EndCommands(VkCommandBuffer commandBuffer);

	int FindQueueGraphicsFamilies(VkPhysicalDevice device);
	int FindQueuePresentFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	int FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	int FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkBool32 FormatIsFilterable(VkFormat format, VkImageTiling tiling);

	VkFormat GetFormat(const METextureLayout& layout);

protected:
	bool InitVulkanInstance();
	bool InitPhysicalDevice();
	bool InitLogicalDevice();
	bool InitCommandPool();
	bool InitDefaultTexture();
	bool InitDepthFormat();

	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

public:
	VkInstance m_VkInstance;
	VkPhysicalDevice m_VkPhysicalDevice;
	VkPhysicalDeviceProperties m_VkPhysicalDeviceProperties;
	VkDevice m_VkDevice;
	VkQueue m_VkGraphicsQueue;

	VkCommandPool m_VkCommandPool;

	MVulkanShaderCompiler m_ShaderCompiler;
	MVulkanPipelineManager m_PipelineManager;
	MVulkanObjectDestructor m_ObjectDestructor;
	MVulkanBufferPool m_BufferPool;

	VkFormat m_VkDepthTextureFormat;

	MTexture m_WhiteTexture;
};


#endif


#endif