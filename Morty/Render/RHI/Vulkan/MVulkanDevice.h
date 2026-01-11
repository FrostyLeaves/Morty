/**
 * @File         MVulkanDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "RHI/Shader/MVulkanShaderReflector.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MVulkanPhysicalDevice.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/Vulkan/MVulkanWrapper.h"

#ifdef MORTY_WIN

#include "vulkan/vulkan_core.h"

#endif

#include "Basic/MTexture.h"
#include "RHI/Shader/MVulkanShaderCompiler.h"
#include "RHI/Vulkan/MVulkanBufferPool.h"
#include "RHI/Vulkan/MVulkanObjectRecycleBin.h"
#include "RHI/Vulkan/MVulkanPipelineManager.h"
#include "Slang/MSlangCompiler.h"

namespace morty
{
class MBuffer;
class IShaderProgram;
class MRenderCommandVulkan;
class MVulkanPrimaryRenderCommand;
class MVulkanSecondaryRenderCommand;

class MORTY_API MVulkanDevice : public MIDevice
{
public:
    explicit MVulkanDevice();

    bool Initialize() override;

    void Release() override;

    void GenerateBuffer(MBuffer* buffer, const MByte* initialData, const size_t& unDataSize) override;

    void DestroyBuffer(MBuffer* buffer) override;

    void
    UploadBuffer(MBuffer* buffer, const size_t& unBeginOffset, const MByte* data, const size_t& unDataSize) override;

    void DownloadBuffer(MBuffer* buffer, MByte* outputData, const size_t& nSize) override;

    void GenerateTexture(MTexture* texture, const std::vector<std::vector<MByte>>& buffer) override;

    void DestroyTexture(MTexture* texture) override;

    bool CompileShader(MShader* pShader) override;

    void CleanShader(MShader* pShader) override;

    bool SyncParameterSet(MShaderParameterSet* propertyBlock) override;

    bool GenerateShaderParameterSet(MShaderParameterSet* pParameterSet) override;

    void DestroyShaderParameterSet(MShaderParameterSet* pParameterSet) override;

    bool GenerateShaderParamBuffer(MShaderUniformParam* pParam) override;

    void DestroyShaderParamBuffer(MShaderUniformParam* pParam) override;

    bool GenerateRenderPass(MRenderPass* pRenderPass) override;

    void DestroyRenderPass(MRenderPass* pRenderPass) override;

    bool GenerateFrameBuffer(MRenderPass* pRenderPass) override;

    void DestroyFrameBuffer(MRenderPass* pRenderPass) override;

    std::shared_ptr<MGraphicsPipeline>
    FindOrCreateGraphicsPipeline(const MMaterialPass* materialPass, const MRenderPass* pRenderPass) override;

    IRenderCommand*              CreateRenderCommand(const MString& strCommandName) override;

    void                         RecoveryRenderCommand(IRenderCommand* pCommand) override;

    bool                         IsFinishedCommand(IRenderCommand* pCommand) override;

    void                         SubmitCommand(IRenderCommand* pCommand) override;

    void                         Update() override;


    //physical interface.
    VkInstance                   GetVkInstance() const;

    const MVulkanPhysicalDevice* GetPhysicalDevice() const;

    bool                         GetDeviceFeatureSupport(MEDeviceFeature feature) const override;

    bool                         MultiDrawIndirectSupport() const;

    int                          FindQueuePresentFamilies(VkSurfaceKHR surface) const;

    VkPhysicalDeviceProperties   GetPhysicalDeviceProperties() const;

    Vector2i                     GetShadingRateTextureTexelSize() const override;

    void                         SetDebugName(uint64_t object, const VkObjectType& type, const char* svDebugName) const;

    void                         TransitionLayoutBarrier(
                                    VkImageMemoryBarrier&   imageMemoryBarrier,
                                    VkImage                 image,
                                    VkImageLayout           oldLayout,
                                    VkImageLayout           newLayout,
                                    VkImageSubresourceRange subresourceRange
                            ) const;

    MVulkanObjectRecycleBin*           GetRecycleBin() const;

    VkFormat                           GetFormat(const METextureFormat& layout) const;

    VkImageUsageFlags                  GetUsageFlags(MTexture* texture) const;

    static VkImageAspectFlags          GetAspectFlags(METextureWriteUsage eUsage);

    static VkImageAspectFlags          GetAspectFlags(VkFormat format);

    VkImageAspectFlags                 GetAspectFlags(VkImageLayout layout) const;

    VkImageLayout                      GetImageLayout(MTexture* texture) const;

    VkImageViewType                    GetImageViewType(MTexture* texture) const;

    VkImageCreateFlags                 GetImageCreateFlags(MTexture* texture) const;

    VkImageType                        GetImageType(MTexture* texture) const;

    uint32_t                           GetMipmapCount(MTexture* texture) const;

    uint32_t                           GetBufferBarrierQueueFamily(MEBufferBarrierStage stage) const;

    VkBufferUsageFlags                 GetBufferUsageFlags(MBuffer* buffer) const;

    VkMemoryPropertyFlags              GetMemoryFlags(MBuffer* buffer) const;

    VkFragmentShadingRateCombinerOpKHR GetShadingRateCombinerOp(MEShadingRateCombinerOp op) const;

    void GenerateBuffer(VkCommandBuffer vkCommand, MBuffer* buffer, const MByte* initialData, const size_t& unDataSize);

    void UploadBuffer(
            VkCommandBuffer vkCommand,
            MBuffer*        buffer,
            const size_t&   unBeginOffset,
            const MByte*    data,
            const size_t&   unDataSize
    );

    void GenerateMipmaps(MTexture* texture, const uint32_t& unMipLevels, VkCommandBuffer buffer = VK_NULL_HANDLE);

    bool GenerateBuffer(
            VkDeviceSize          size,
            VkBufferUsageFlags    usage,
            VkMemoryPropertyFlags properties,
            VkBuffer&             buffer,
            VkDeviceMemory&       bufferMemory
    );

    void                              DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    MVulkanSecondaryRenderCommand*    CreateChildCommand(MVulkanPrimaryRenderCommand* pParentCommand);


    std::tuple<VkImageView, Vector2i> CreateFrameBufferViewFromRenderTarget(MRenderTarget& renderTarget);

    bool                              CompileShaderHlsl(MShader* pShader, std::vector<uint32_t>& spirv);

    bool                              CompileShaderSlang(MShader* pShader, std::vector<uint32_t>& spirv);

    void                              UpdateShaderParam(MShaderUniformParam* param);

protected:
    void CopyBuffer(
            VkBuffer        srcBuffer,
            VkBuffer        dstBuffer,
            VkBufferCopy    region,
            VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE
    );

    void CopyImageBuffer(
            VkBuffer        srcBuffer,
            VkImage         image,
            const uint32_t& width,
            const uint32_t& height,
            const uint32_t& unCount
    );

    void CopyImageBuffer(MTexture* pSource, MTexture* pDestination, VkCommandBuffer buffer = VK_NULL_HANDLE);

    void TransitionImageLayout(
            VkImage                 image,
            VkImageLayout           oldLayout,
            VkImageLayout           newLayout,
            VkImageSubresourceRange subresourceRange
    );

    void TransitionImageLayout(
            VkCommandBuffer         commandBuffer,
            VkImage                 image,
            VkImageLayout           oldLayout,
            VkImageLayout           newLayout,
            VkImageSubresourceRange subresourceRange
    );

    void CreateImage(
            uint32_t              nWidth,
            uint32_t              nHeight,
            uint32_t              nDepth,
            const uint32_t&       unMipmap,
            const uint32_t&       unLayerCount,
            VkFormat              format,
            VkImageTiling         tiling,
            VkImageUsageFlags     usage,
            VkMemoryPropertyFlags properties,
            VkImageLayout         defaultLayout,
            VkImage&              image,
            VkDeviceMemory&       imageMemory,
            VkImageCreateFlags    createFlag,
            VkImageType           imageType
    );

    VkImageView CreateImageView(
            VkImage                image,
            VkFormat               format,
            VkImageAspectFlags     aspectFlags,
            const uint32_t&        unMipmap,
            const uint32_t&        unLayerCount,
            const VkImageViewType& eViewType
    );

    VkImageView CreateImageView(
            VkImage                image,
            VkFormat               format,
            VkImageAspectFlags     aspectFlags,
            const uint32_t&        unBaseMipmap,
            const uint32_t&        unMipmapCount,
            const uint32_t&        unLayerCount,
            const VkImageViewType& eViewType
    );

    void            CheckFrameFinish();

    void            WaitFrameFinish();

    VkCommandBuffer BeginCommands();

    void            EndCommands(VkCommandBuffer commandBuffer);


    bool            InitLogicalDevice();

    bool            InitCommandPool();

    bool            InitSampler();

    bool            InitDescriptorPool();

    void            CreateRecycleBin();

public:
    VkDevice         m_vkDevice               = VK_NULL_HANDLE;
    VkQueue          m_vkTemporaryQueue       = VK_NULL_HANDLE;
    VkQueue          m_vkPresetQueue          = VK_NULL_HANDLE;
    VkCommandPool    m_vkPresetCommandPool    = VK_NULL_HANDLE;
    VkCommandPool    m_vkTemporaryCommandPool = VK_NULL_HANDLE;
    VkDescriptorPool m_vkDescriptorPool       = VK_NULL_HANDLE;

    VkSampler        m_vkLinearSampler  = VK_NULL_HANDLE;
    VkSampler        m_vkNearestSampler = VK_NULL_HANDLE;

public:
    std::unique_ptr<MVulkanShaderCompiler> m_ShaderCompiler;
    MVulkanShaderReflector                 m_ShaderReflector;
    MVulkanPipelineManager                 m_PipelineManager;
    MVulkanBufferPool                      m_BufferPool;


    struct MVkFrameData {
        MVulkanObjectRecycleBin*           pRecycleBin;
        std::vector<MRenderCommandVulkan*> vCommand;
    };

    MVulkanObjectRecycleBin*               m_recycleBin = nullptr;

    std::map<uint32_t, MVkFrameData>       m_frameData;

    uint32_t                               m_unFrameCount = 0;


    std::unique_ptr<MVulkanPhysicalDevice> m_physicalDevice = nullptr;
};
}// namespace morty

#endif
