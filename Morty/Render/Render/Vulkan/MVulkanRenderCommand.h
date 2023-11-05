/**
 * @File         MVulkanRenderCommand
 * 
 * @Created      2021-07-14 18:22:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Render/Vulkan/MVulkanDevice.h"
#include "Render/MRenderCommand.h"

class MTexture;
class MORTY_API MVulkanRenderCommand : public MIRenderCommand
{
public:
    explicit MVulkanRenderCommand() = default;
    ~MVulkanRenderCommand() override = default;

public:

    void SetViewport(const MViewportInfo& viewport) override;
    void SetScissor(const MScissorInfo& scissor) override;

	void RenderCommandBegin() override;
	void RenderCommandEnd() override;

	void BeginRenderPass(MRenderPass* pRenderPass) override;
	void NextSubPass() override;
	void EndRenderPass() override;

	void DrawMesh(MIMesh* pMesh) override;
	void DrawMesh(MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset) override;
	void DrawMesh(const MBuffer* pVertexBuffer, const MBuffer* pIndexBuffer, const size_t nVertexOffset, const size_t nIndexOffset, const size_t nIndexCount) override;
    void DrawIndexedIndirect(const MBuffer* pVertexBuffer, const MBuffer* pIndexBuffer, const MBuffer* pCommandsBuffer, const size_t& offset, const size_t& count) override;


	bool SetUseMaterial(std::shared_ptr<MMaterial> pMaterial) override;
	bool SetGraphPipeline(std::shared_ptr<MMaterial> pMaterial) override;
	void SetShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) override;

	bool DispatchComputeJob(MComputeDispatcher* pComputeDispatcher, const uint32_t& nGroupX, const uint32_t& nGroupY, const uint32_t& nGroupZ) override;

	bool AddRenderToTextureBarrier(const std::vector<MTexture*> vTextures, METextureBarrierStage dstStage) override;

	bool AddBufferMemoryBarrier(const std::vector<const MBuffer*> vBuffers, MEBufferBarrierStage srcStage, MEBufferBarrierStage dstStage) override;

	bool DownloadTexture(MTexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) override;
	bool CopyImageBuffer(MTexture* pSource, MTexture* pDest) override;
	void UpdateMipmaps(MTexture* pBuffer) override;
	void ResetBuffer(const MBuffer* pBuffer) override;
	void FillTexture(MTexture* pBuffer, MColor color) override;

	void addFinishedCallback(std::function<void()> func) override;

	void UpdateBuffer(MBuffer* pBuffer, const MByte* data, const size_t& size);

	void UpdateShaderParam(std::shared_ptr<MShaderConstantParam> param);

	void SetTextureLayout(const std::vector<MTexture*>& vTextures, VkImageLayout newLayout);

protected:
	VkImageLayout GetTextureBarrierLayout(METextureBarrierStage stage) const;
	VkAccessFlags GetBufferBarrierAccessFlag(MEBufferBarrierStage stage) const;
	uint32_t GetBufferBarrierQueueFamily(MEBufferBarrierStage stage) const;
	VkPipelineStageFlags GetBufferBarrierPipelineStage(MEBufferBarrierStage stage) const;
	VkPipelineStageFlags GetTextureBarrierPipelineStage(METextureBarrierStage stage) const;
public:

	MVulkanDevice* m_pDevice = nullptr;

	std::shared_ptr<MPipeline> pUsingPipeline = nullptr;
	const MBuffer* pUsingVertex = nullptr;
	const MBuffer* pUsingIndex = nullptr;
	std::stack<MRenderPassStage> m_vRenderPassStages;

	VkCommandBuffer m_VkCommandBuffer = VK_NULL_HANDLE;

	std::map<MTexture*, VkImageLayout> m_tTextureLayout;

	std::vector<std::function<void()>> m_aRenderFinishedCallback = {};
	
};

class MORTY_API MVulkanSecondaryRenderCommand : public MVulkanRenderCommand
{

};

class MORTY_API MVulkanPrimaryRenderCommand : public MVulkanRenderCommand
{
public:
	MVulkanPrimaryRenderCommand();

	bool IsFinished() override { return m_bFinished; }
	void CheckFinished() override;

	MIRenderCommand* CreateChildCommand() override;
	MIRenderCommand* GetChildCommand(const size_t& nIndex) override;
	void ExecuteChildCommand() override;

public:

	VkFence m_VkRenderFinishedFence; // fence --> CPU
	VkSemaphore m_VkRenderFinishedSemaphore; // semaphore --> GPU

	std::vector<VkSemaphore> m_vRenderWaitSemaphore;

	std::vector<MVulkanSecondaryRenderCommand*> m_vSecondaryCommand;

	bool m_bFinished;
};

//TODO objectDestructor.FrameFinished;
//TODO RenderFinishedCallback;

#endif
