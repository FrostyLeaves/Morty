/**
 * @File         MRenderCommandVulkan
 * 
 * @Created      2021-07-14 18:22:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/IRenderCommand.h"
#include "RHI/Vulkan/MVulkanCommandExecuteTable.h"
#include "RHI/Vulkan/MVulkanDevice.h"

namespace morty
{
class MTexture;
struct MBufferRHIVulkan;
class MORTY_API MRenderCommandVulkan : public IRenderCommand
{
    friend class MVulkanCommandExecuteTable;

public:
    explicit MRenderCommandVulkan() = default;

    ~MRenderCommandVulkan() override = default;

public:
    void           RenderCommandBegin() override;

    void           RenderCommandEnd() override;

    MRenderPassCmd BeginRenderPass(MRenderPass* renderPass) override;
    void           EndRenderPass(const MRenderPassCmd& command) override;

    bool           DispatchComputeJob(
                      MComputeDispatcher* pComputeDispatcher,
                      const MStringId&    entryName,
                      const uint32_t&     nGroupX,
                      const uint32_t&     nGroupY,
                      const uint32_t&     nGroupZ
              ) override;

    bool AddBufferMemoryBarrier(
            const std::vector<const MBufferRHI*>& vBuffers,
            MEBufferBarrierStage                  srcStage,
            MEBufferBarrierStage                  dstStage
    ) override;

    bool DownloadTexture(
            MTexture*                                                         texture,
            const uint32_t&                                                   unMipIdx,
            const std::function<void(void* pImageData, const Vector2& size)>& callback
    ) override;

    bool CopyImageBuffer(MTexture* pSource, MTexture* pDest) override;

    void addFinishedCallback(std::function<void()> func) override;

    void UpdateShaderParam(MShaderUniformParam* param);

    void SetTextureLayout(const std::vector<MTexture*>& vTextures, const std::vector<VkImageLayout>& newLayouts);

protected:
    bool          AddRenderToTextureBarrier(const std::vector<MTexture*>& vTextures, METextureBarrierStage dstStage);

    VkImageLayout GetTextureBarrierLayout(MTexture* texture, METextureBarrierStage stage) const;

    [[nodiscard]] VkAccessFlags        GetBufferBarrierAccessFlag(MEBufferBarrierStage stage) const;

    [[nodiscard]] VkPipelineStageFlags GetBufferBarrierPipelineStage(MEBufferBarrierStage stage) const;

    [[nodiscard]] VkPipelineStageFlags GetTextureBarrierPipelineStage(METextureBarrierStage stage) const;

private:
    void                              InternalBeginRenderPass(MRenderPass* renderpass);
    void                              InternalEndRenderPass();
    void                              SetViewport(const MSetViewportCmd* viewport) const;
    void                              SetScissor(const MSetScissorCmd* scissor) const;
    void                              DrawMesh(const MDrawMeshCmd* cmd);
    void                              DrawIndexedIndirect(const MDrawIndexedIndirectCmd* cmd);
    void                              SetGraphPipeline(const MSetGraphPipelineCmd* cmd);
    void                              SetShaderParameterSet(const MSetShaderParameterSetCmd* cmd);
    void                              AddBarrierForPixelSample(const MSetShaderParameterSetCmd* cmd);
    void                              NextSubPass(const MNextSubPassCmd* cmd);
    void                              SetShadingRate(const MSetShadingRateCmd* cmd);


    static MVulkanCommandExecuteTable m_executeTable;

public:
    MVulkanDevice*                                    m_device     = nullptr;
    const MBufferRHIVulkan*                           pUsingVertex = nullptr;
    const MBufferRHIVulkan*                           pUsingIndex  = nullptr;

    VkCommandBuffer                                   m_vkCommandBuffer = VK_NULL_HANDLE;

    std::map<MTexture*, VkImageLayout>                m_textureLayout;

    std::vector<std::function<void()>>                m_renderFinishedCallback = {};

    std::vector<std::shared_ptr<MShaderParameterSet>> m_propertyBlockStack;
};

class MORTY_API MVulkanSecondaryRenderCommand : public MRenderCommandVulkan
{
};

class MORTY_API MVulkanPrimaryRenderCommand : public MRenderCommandVulkan
{
public:
    MVulkanPrimaryRenderCommand();

    bool            IsFinished() override { return m_finished; }

    void            MarkFinished();

    void            OnCommandFinished() override;

    IRenderCommand* CreateChildCommand();

    IRenderCommand* GetChildCommand(const size_t& nIndex);

    void            ExecuteChildCommand();

public:
    VkFence                                     m_vkRenderFinishedFence;    // fence --> CPU
    VkSemaphore                                 m_vkRenderFinishedSemaphore;// semaphore --> GPU

    std::vector<VkSemaphore>                    m_renderWaitSemaphore;

    std::vector<MVulkanSecondaryRenderCommand*> m_secondaryCommand;

    bool                                        m_finished;
};

//TODO objectDestructor.FrameFinished;
//TODO RenderFinishedCallback;

}// namespace morty

#endif
