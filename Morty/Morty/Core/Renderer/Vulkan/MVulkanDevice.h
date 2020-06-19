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

#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"


class MORTY_CLASS MVulkanDevice : public MIDevice
{
public:
    MVulkanDevice();
    virtual ~MVulkanDevice();

public:

public:
	virtual bool Initialize() override;
	virtual void Release() override;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) override;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) override;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) override;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGenerateMipmap) override;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap) override;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) override;

	virtual void GenerateRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer, const MERenderTextureType& eType, const unsigned int& unWidth, const unsigned& unHeight) override;
	virtual void DestroyRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer) override;

	virtual void GenerateDepthTexture(MDepthTextureBuffer** ppTextureBuffer, const unsigned int& unWidth, const unsigned int& unHeight) override;
	virtual void DestroyDepthTexture(MDepthTextureBuffer** ppTextureBuffer) override;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType, const MShaderMacro& macro) override;
	virtual void CleanShader(MShaderBuffer** ppShaderBuffer) override;

	virtual bool GenerateRenderTarget(MIRenderTarget* pRenderTarget, unsigned int nWidth, unsigned int nHeight) override;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) override;

	virtual bool GenerateRenderTarget(MTextureRenderTarget* pRenderTarget, unsigned int nWidth, unsigned int nHeight) override;
	virtual void DestroyRenderTarget(MTextureRenderTarget* pRenderTarget) override;

	virtual bool GenerateShaderParamBuffer(MShaderParam* pParam) override;
	virtual void DestroyShaderParamBuffer(MShaderParam* pParam) override;


	VkPhysicalDevice GetPhysicalDevice() { return m_VkPhysicalDevice; }

	bool GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	int FindQueueGraphicsFamilies(VkPhysicalDevice device);
	int FindQueuePresentFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	int FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
protected:
	bool InitVulkanInstance();
	bool InitPhysicalDevice();
	bool InitLogicalDevice();

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
	VkDevice m_VkDevice;
	VkQueue m_VkGraphicsQueue;


	int m_nBufferNum;
};


#endif


#endif