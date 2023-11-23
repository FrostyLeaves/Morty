/**
 * @File         MVulkanDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MVulkanShaderReflector.h"
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
    explicit MVulkanDevice();
    virtual ~MVulkanDevice();

public:
	virtual bool Initialize() override;
	virtual void Release() override;

public:
	virtual void GenerateBuffer(MBuffer* pBuffer, const MByte* initialData, const size_t& unDataSize) override;
	virtual void DestroyBuffer(MBuffer* pBuffer) override;
	virtual void UploadBuffer(MBuffer* pBuffer, const size_t& unBeginOffset, const MByte* data, const size_t& unDataSize) override;
	virtual void DownloadBuffer(MBuffer* pBuffer, MByte* outputData, const size_t& nSize) override;

	virtual void GenerateTexture(MTexture* pTexture, const MByte* pData = nullptr) override;
	virtual void DestroyTexture(MTexture* pTexture) override;

	virtual bool CompileShader(MShader* pShader) override;
	virtual void CleanShader(MShader* pShader) override;

	virtual bool GenerateShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) override;
	virtual void DestroyShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) override;

	virtual bool GenerateShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) override;
	virtual void DestroyShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) override;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) override;
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) override;

	virtual bool GenerateFrameBuffer(MRenderPass* pRenderPass) override;
	virtual void DestroyFrameBuffer(MRenderPass* pRenderPass) override;

	virtual bool RegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher) override;
	virtual bool UnRegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher) override;

	virtual MIRenderCommand* CreateRenderCommand(const MString& strCommandName) override;
	virtual void RecoveryRenderCommand(MIRenderCommand* pCommand) override;

	virtual bool IsFinishedCommand(MIRenderCommand* pCommand) override;

	virtual void SubmitCommand(MIRenderCommand* pCommand) override;

	virtual void Update() override;

	bool GetDeviceFeatureSupport(MEDeviceFeature feature) const override;

public:

	VkPhysicalDevice GetPhysicalDevice() { return m_VkPhysicalDevice; }

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkBufferCopy region);
	void CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height, const uint32_t& unCount);

	void CopyImageBuffer(MTexture* pSource, MTexture* pDestination, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void GenerateMipmaps(MTexture* pBuffer, const uint32_t& unMipLevels, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
	void TransitionImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unMipmap, const uint32_t& unLayerCount, const VkImageViewType& eViewType);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unBaseMipmap, const uint32_t& unMipmapCount, const uint32_t& unLayerCount, const VkImageViewType& eViewType);

	void CreateImage(uint32_t nWidth, uint32_t nHeight, uint32_t nDepth, const uint32_t& unMipmap, const uint32_t& unLayerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout defaultLayout, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlag, VkImageType imageType);

	VkBufferUsageFlags GetBufferUsageFlags(MBuffer* pBuffer) const;
	VkMemoryPropertyFlags GetMemoryFlags(MBuffer* pBuffer) const;
	bool GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	MVulkanSecondaryRenderCommand* CreateChildCommand(MVulkanPrimaryRenderCommand* pParentCommand);

	void CheckFrameFinish();
	void WaitFrameFinish();

	VkCommandBuffer BeginCommands();
	void EndCommands(VkCommandBuffer commandBuffer);

	int FindQueueGraphicsFamilies(VkPhysicalDevice device);
	int FindQueuePresentFamilies(VkSurfaceKHR surface);
	int FindQueueComputeFamilies(VkPhysicalDevice device);
	bool MultiDrawIndirectSupport() const;

	int FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat FindSupportedFormat(const std::set<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

	VkBool32 FormatIsFilterable(VkFormat format, VkImageTiling tiling);

	VkFormat GetFormat(const METextureLayout& layout);
	VkImageUsageFlags GetUsageFlags(MTexture* pTexture);
	VkImageAspectFlags GetAspectFlags(MTexture* pTexture);
	VkImageAspectFlags GetAspectFlags(VkFormat format);
	VkImageAspectFlags GetAspectFlags(VkImageLayout layout);
	VkImageLayout GetImageLayout(MTexture* pTexture);
	VkImageViewType GetImageViewType(MTexture* pTexture);
	VkImageCreateFlags GetImageCreateFlags(MTexture* pTexture);
	VkImageType GetImageType(MTexture* pTexture);
    uint32_t GetMipmapCount(MTexture* pTexture);
    uint32_t GetLayerCount(MTexture* pTexture);

	MVulkanObjectRecycleBin* GetRecycleBin();

	void SetDebugName(uint64_t object, const VkObjectType& type, const char* svDebugName);
	bool CheckVersion(int major, int minor, int patch);

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
	void InitOptionalFeature();

	bool IsDeviceSuitable(VkPhysicalDevice device);
	std::set<MString> GetDeviceOptionFeatureNotSupport(VkPhysicalDevice device);
	std::set<MString> GetNotSupportDeviceExtension(VkPhysicalDevice device, const std::set<MString>& tRequiredExtensions) const;

public:
	VkInstance m_VkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice m_VkPhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures m_VkPhysicalDeviceFeatures;
	VkPhysicalDeviceProperties m_VkPhysicalDeviceProperties = {};
	VkDevice m_VkDevice = VK_NULL_HANDLE;
	VkQueue m_VkGraphicsQueue = VK_NULL_HANDLE;
	std::vector<const char*> m_vEnableDeviceExtensions;

	int m_nGraphicsFamilyIndex = 0;
	int m_nComputeFamilyIndex = 0;

	int m_nVulkanVersionMajor = 0;
	int m_nVulkanVersionMinor = 0;
	int m_nVulkanVersionPatch = 0;

	VkCommandPool m_VkGraphCommandPool;

	MVulkanShaderCompiler m_ShaderCompiler;
	MVulkanShaderReflector m_ShaderReflector;
	MVulkanPipelineManager m_PipelineManager;
	MVulkanBufferPool m_BufferPool;

	VkFormat m_VkDepthTextureFormat = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags m_VkDepthAspectFlags = VK_IMAGE_LAYOUT_UNDEFINED;

	std::shared_ptr<MTexture> m_ShaderDefaultTexture;
	std::shared_ptr<MTexture> m_ShaderDefaultTextureCube;
	std::shared_ptr<MTexture> m_ShaderDefaultTextureArray;
	std::shared_ptr<MTexture> m_ShaderDefaultTexture3D;

	VkSampler m_VkLinearSampler = VK_NULL_HANDLE;
	VkSampler m_VkNearestSampler = VK_NULL_HANDLE;

	VkDescriptorPool m_VkDescriptorPool = VK_NULL_HANDLE;

	//optional extensions
	VkPhysicalDeviceConservativeRasterizationPropertiesEXT m_VkConservativeRasterProps{};
	
#if MORTY_DEBUG
	VkDebugUtilsMessengerEXT m_VkDebugUtilsMessenger;
#endif
public:

	struct MVkFrameData
	{
		MVulkanObjectRecycleBin* pRecycleBin;
		std::vector<MVulkanRenderCommand*> vCommand;
	};

	MVulkanObjectRecycleBin* m_pRecycleBin = nullptr;

	MVulkanObjectRecycleBin* m_pDefaultRecycleBin = nullptr;
	std::map<uint32_t, MVkFrameData> m_tFrameData;

	uint32_t m_unFrameCount = 0;

	std::set<MEDeviceFeature> m_tDisableFeature;

public:

	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet = VK_NULL_HANDLE;
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = VK_NULL_HANDLE;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2 = VK_NULL_HANDLE;
};


#endif
