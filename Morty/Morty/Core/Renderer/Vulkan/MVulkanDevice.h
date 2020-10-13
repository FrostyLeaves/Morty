/**
 * @File         MVulkanDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       Pobrecito
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
#include "MVulkanUniformBufferPool.h"

#include "MTexture.h"

class MORTY_CLASS MVulkanDevice : public MIDevice
{
public:
    MVulkanDevice();
    virtual ~MVulkanDevice();

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void Tick(const float& fDelta) override;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) override;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) override;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) override;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGenerateMipmap) override;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap) override {}
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) override;

	virtual bool GenerateRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer, const METextureLayout& eType, const uint32_t& unWidth, const unsigned& unHeight) override;
	virtual void DestroyRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer) override;

	virtual void GenerateDepthTexture(MDepthTextureBuffer** ppTextureBuffer, const uint32_t& unWidth, const uint32_t& unHeight) override;
	virtual void DestroyDepthTexture(MDepthTextureBuffer** ppTextureBuffer) override;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro) override;
	virtual void CleanShader(MShaderBuffer** ppShaderBuffer) override;

	virtual bool GenerateShaderParamSet(MShaderParamSet* pParamSet) override;
	virtual void DestroyShaderParamSet(MShaderParamSet* pParamSet) override;

	virtual bool GenerateRenderTarget(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget) override;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) override;

	virtual bool GenerateShaderParamBuffer(MShaderConstantParam* pParam) override;
	virtual void DestroyShaderParamBuffer(MShaderConstantParam* pParam) override;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget) override;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) override;

	virtual bool GenerateRenderTargetView(MRenderTextureBuffer* pTextureBuffer) override;
	virtual void DestroyRenderTargetView(MRenderTextureBuffer* pTextureBuffer) override;

	virtual void RegisterMaterial(MMaterial* pMaterial) override;
	virtual void UnRegisterMaterial(MMaterial* pMaterial) override;

	VkPhysicalDevice GetPhysicalDevice() { return m_VkPhysicalDevice; }

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void CreateImage(const uint32_t& unWidth, const uint32_t& unHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkDescriptorSet CreateMaterialDescriptorSet(VkDescriptorSetLayout& vkDescriptorSetLayout);

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
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
	PFN_vkQueuePresentKHR QueuePresentKHR;

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
	MVulkanUniformBufferPool m_DynamicUniformBufferPool;

	VkFormat m_VkDepthTextureFormat;

	MTexture m_WhiteTexture;
};


#endif


#endif