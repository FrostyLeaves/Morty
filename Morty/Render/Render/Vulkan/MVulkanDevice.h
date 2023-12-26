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
#include "MVulkanPhysicalDevice.h"

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

	bool Initialize() override;
	void Release() override;

	void GenerateBuffer(MBuffer* pBuffer, const MByte* initialData, const size_t& unDataSize) override;
	void DestroyBuffer(MBuffer* pBuffer) override;
	void UploadBuffer(MBuffer* pBuffer, const size_t& unBeginOffset, const MByte* data, const size_t& unDataSize) override;
	void DownloadBuffer(MBuffer* pBuffer, MByte* outputData, const size_t& nSize) override;

	void GenerateTexture(MTexture* pTexture, const MByte* pData = nullptr) override;
	void DestroyTexture(MTexture* pTexture) override;

	bool CompileShader(MShader* pShader) override;
	void CleanShader(MShader* pShader) override;

	bool GenerateShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) override;
	void DestroyShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) override;

	bool GenerateShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) override;
	void DestroyShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam) override;

	bool GenerateRenderPass(MRenderPass* pRenderPass) override;
	void DestroyRenderPass(MRenderPass* pRenderPass) override;

	bool GenerateFrameBuffer(MRenderPass* pRenderPass) override;
	void DestroyFrameBuffer(MRenderPass* pRenderPass) override;

	MIRenderCommand* CreateRenderCommand(const MString& strCommandName) override;
	void RecoveryRenderCommand(MIRenderCommand* pCommand) override;

	bool IsFinishedCommand(MIRenderCommand* pCommand) override;

	void SubmitCommand(MIRenderCommand* pCommand) override;

	void Update() override;


//physical interface.
	VkInstance GetVkInstance() const;
	const MVulkanPhysicalDevice* GetPhysicalDevice() const;
	bool GetDeviceFeatureSupport(MEDeviceFeature feature) const override;
	bool MultiDrawIndirectSupport() const;
	int FindQueuePresentFamilies(VkSurfaceKHR surface) const;
	VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const;
	Vector2i GetShadingRateTextureTexelSize() const override;

	void SetDebugName(uint64_t object, const VkObjectType& type, const char* svDebugName) const;
	void TransitionLayoutBarrier(VkImageMemoryBarrier& imageMemoryBarrier, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) const;
	MVulkanObjectRecycleBin* GetRecycleBin() const;

	VkFormat GetFormat(const METextureLayout& layout) const;
	VkImageUsageFlags GetUsageFlags(MTexture* pTexture) const;
	VkImageAspectFlags GetAspectFlags(METextureWriteUsage eUsage) const;
	VkImageAspectFlags GetAspectFlags(VkFormat format) const;
	VkImageAspectFlags GetAspectFlags(VkImageLayout layout) const;
	VkImageLayout GetImageLayout(MTexture* pTexture) const;
	VkImageViewType GetImageViewType(MTexture* pTexture) const;
	VkImageCreateFlags GetImageCreateFlags(MTexture* pTexture) const;
	VkImageType GetImageType(MTexture* pTexture) const;
	uint32_t GetMipmapCount(MTexture* pTexture) const;
	uint32_t GetLayerCount(MTexture* pTexture) const;
	uint32_t GetBufferBarrierQueueFamily(MEBufferBarrierStage stage) const;
	VkBufferUsageFlags GetBufferUsageFlags(MBuffer* pBuffer) const;
	VkMemoryPropertyFlags GetMemoryFlags(MBuffer* pBuffer) const;
	VkFragmentShadingRateCombinerOpKHR GetShadingRateCombinerOp(MEShadingRateCombinerOp op) const;

	void GenerateMipmaps(MTexture* pBuffer, const uint32_t& unMipLevels, VkCommandBuffer buffer = VK_NULL_HANDLE);
	bool GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	MVulkanSecondaryRenderCommand* CreateChildCommand(MVulkanPrimaryRenderCommand* pParentCommand);


	std::tuple<VkImageView, Vector2i> CreateFrameBufferViewFromRenderTarget(MRenderTarget& renderTarget);

protected:
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkBufferCopy region);
	void CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height, const uint32_t& unCount);
	void CopyImageBuffer(MTexture* pSource, MTexture* pDestination, VkCommandBuffer buffer = VK_NULL_HANDLE);

	void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

	void CreateImage(uint32_t nWidth, uint32_t nHeight, uint32_t nDepth, const uint32_t& unMipmap, const uint32_t& unLayerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout defaultLayout, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlag, VkImageType imageType);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unMipmap, const uint32_t& unLayerCount, const VkImageViewType& eViewType);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unBaseMipmap, const uint32_t& unMipmapCount, const uint32_t& unLayerCount, const VkImageViewType& eViewType);

	void CheckFrameFinish();
	void WaitFrameFinish();

	VkCommandBuffer BeginCommands();
	void EndCommands(VkCommandBuffer commandBuffer);


	bool InitLogicalDevice();
	bool InitCommandPool();
	bool InitDefaultTexture();
	bool InitSampler();
	bool InitDescriptorPool();
	void CreateRecycleBin();
public:
	VkDevice m_VkDevice = VK_NULL_HANDLE;
	VkQueue m_VkGraphicsQueue = VK_NULL_HANDLE;
	VkCommandPool m_VkGraphCommandPool = VK_NULL_HANDLE;
	VkDescriptorPool m_VkDescriptorPool = VK_NULL_HANDLE;

	std::shared_ptr<MTexture> m_ShaderDefaultTexture = nullptr;
	std::shared_ptr<MTexture> m_ShaderDefaultTextureCube = nullptr;
	std::shared_ptr<MTexture> m_ShaderDefaultTextureArray = nullptr;
	std::shared_ptr<MTexture> m_ShaderDefaultTexture3D = nullptr;

	VkSampler m_VkLinearSampler = VK_NULL_HANDLE;
	VkSampler m_VkNearestSampler = VK_NULL_HANDLE;

public:

	MVulkanShaderCompiler m_ShaderCompiler;
	MVulkanShaderReflector m_ShaderReflector;
	MVulkanPipelineManager m_PipelineManager;
	MVulkanBufferPool m_BufferPool;


	struct MVkFrameData
	{
		MVulkanObjectRecycleBin* pRecycleBin;
		std::vector<MVulkanRenderCommand*> vCommand;
	};

	MVulkanObjectRecycleBin* m_pRecycleBin = nullptr;

	std::map<uint32_t, MVkFrameData> m_tFrameData;

	uint32_t m_unFrameCount = 0;


	std::unique_ptr<MVulkanPhysicalDevice> m_pPhysicalDevice = nullptr;
};


#endif
