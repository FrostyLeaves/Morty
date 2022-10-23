/**
 * @File         MVulkanDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVULKANDEVICE_H_
#define _M_MVULKANDEVICE_H_
#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Render/MIDevice.h"

#include <vector>
#include "Render/Vulkan/MVulkanWrapper.h"

#ifdef MORTY_WIN
#include "vulkan/vulkan_core.h"
#endif

#include "Render/Vulkan/MVulkanObjectRecycleBin.h"
#include "Render/Vulkan/MVulkanShaderCompiler.h"
#include "Render/Vulkan/MVulkanPipelineManager.h"
#include "Render/Vulkan/MVulkanBufferPool.h"

#include "Basic/MTexture.h"

class MBuffer;
class MVulkanRenderCommand;
class MVulkanPrimaryRenderCommand;
class MVulkanSecondaryRenderCommand;
class MORTY_API MVulkanDevice : public MIDevice
{
public:
    MVulkanDevice();
    virtual ~MVulkanDevice();

public:
	virtual bool Initialize() override;
	virtual void Release() override;

public:
	virtual void GenerateBuffer(MBuffer* pBuffer) override;
	virtual void DestroyBuffer(MBuffer* pBuffer) override;
	virtual void UploadBuffer(MBuffer* pBuffer) override;

	virtual void GenerateTexture(MTexture* pTexture, MByte* pData = nullptr) override;
	virtual void DestroyTexture(MTexture* pTexture) override;

	virtual bool CompileShader(MShader* pShader) override;
	virtual void CleanShader(MShader* pShader) override;

	virtual bool GenerateShaderParamSet(MShaderParamSet* pParamSet) override;
	virtual void DestroyShaderParamSet(MShaderParamSet* pParamSet) override;

	virtual bool GenerateShaderParamBuffer(MShaderConstantParam* pParam) override;
	virtual void DestroyShaderParamBuffer(MShaderConstantParam* pParam) override;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) override;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) override;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) override;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) override;

	virtual bool RegisterMaterial(std::shared_ptr<MMaterial> pMaterial) override;
	virtual bool UnRegisterMaterial(std::shared_ptr<MMaterial> pMaterial) override;

	virtual MIRenderCommand* CreateRenderCommand(const MString& strCommandName) override;
	virtual void RecoveryRenderCommand(MIRenderCommand* pCommand) override;

	virtual bool IsFinishedCommand(MIRenderCommand* pCommand) override;

	virtual void SubmitCommand(MIRenderCommand* pCommand) override;

	virtual void Update() override;

public:

	VkPhysicalDevice GetPhysicalDevice() { return m_VkPhysicalDevice; }

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height, const uint32_t& unCount);

	void CopyImageBuffer(MTexture* pSource, MTexture* pDestination, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void GenerateMipmaps(MTexture* pBuffer, const uint32_t& unMipLevels, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
	void TransitionImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unMipmap, const uint32_t& unLayerCount, const VkImageViewType& eViewType);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unBaseMipmap, const uint32_t& unMipmapCount, const uint32_t& unLayerCount, const VkImageViewType& eViewType);

	void CreateImage(const uint32_t& unWidth, const uint32_t& unHeight, const uint32_t& unMipmap, const uint32_t& unLayerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout defaultLayout, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlag, VkImageType imageType);

	bool GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);


	MVulkanSecondaryRenderCommand* CreateChildCommand(MVulkanPrimaryRenderCommand* pParentCommand);

	void CheckFrameFinish();
	void WaitFrameFinish();

	VkCommandBuffer BeginCommands();
	void EndCommands(VkCommandBuffer commandBuffer);

	int FindQueueGraphicsFamilies(VkPhysicalDevice device);
	int FindQueuePresentFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	int FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	int FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkBool32 FormatIsFilterable(VkFormat format, VkImageTiling tiling);

	VkFormat GetFormat(const METextureLayout& layout);
	VkImageUsageFlags GetUsageFlags(MTexture* pTexture);
	VkImageAspectFlags GetAspectFlags(MTexture* pTexture);
	VkImageLayout GetImageLayout(MTexture* pTexture);
	VkImageViewType GetImageViewType(MTexture* pTexture);
	VkImageCreateFlags GetImageCreateFlags(MTexture* pTexture);
	VkImageType GetImageType(MTexture* pTexture);
	int GetMipmapCount(MTexture* pTexture);
	int GetLayerCount(MTexture* pTexture);

	MVulkanObjectRecycleBin* GetRecycleBin();

	void SetDebugName(uint64_t object, const VkObjectType& type, const char* svDebugName);

protected:
	bool InitVulkanInstance();
	bool InitPhysicalDevice();
	bool InitLogicalDevice();
	bool InitCommandPool();
	bool InitDefaultTexture();
	bool InitDepthFormat();
	bool InitSampler();
	bool InitializeRecycleBin();
	bool InitDescriptorPool();


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
	MVulkanBufferPool m_BufferPool;

	VkFormat m_VkDepthTextureFormat;

	MTexture m_ShaderDefaultTexture;
	MTexture m_ShaderDefaultTextureCube;
	MTexture m_ShaderDefaultTextureArray;

	VkSampler m_VkLinearSampler;
	VkSampler m_VkNearestSampler;

	VkDescriptorPool m_VkDescriptorPool;


#ifdef _DEBUG
	VkDebugUtilsMessengerEXT m_VkDebugUtilsMessenger;
#endif
public:

	struct MVkFrameData
	{
		MVulkanObjectRecycleBin* pRecycleBin;
		std::vector<MVulkanRenderCommand*> vCommand;
	};

	MVulkanObjectRecycleBin* m_pRecycleBin;

	MVulkanObjectRecycleBin* m_pDefaultRecycleBin;
	std::map<uint32_t, MVkFrameData> m_tFrameData;

	uint32_t m_unFrameCount;

public:

	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet;
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
};


#endif


#endif