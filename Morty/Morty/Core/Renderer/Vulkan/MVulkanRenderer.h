/**
 * @File         MVulkanRenderer
 * 
 * @Created      2020-06-18 15:18:31
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANRENDERER_H_
#define _M_MVULKANRENDERER_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include <array>

#include "MIRenderer.h"

class MRenderPass;
class MVulkanDevice;
class MMaterialGroup;
class MORTY_API MVulkanRenderer : public MIRenderer
{
public:
    MVulkanRenderer(MVulkanDevice* pDevice);
    virtual ~MVulkanRenderer();

public:

	virtual uint32_t GetFrameIndex() override { return m_unFrameIndex; }

	virtual void NewRenderFrame() override;

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) override;

	virtual void RenderBegin(MIRenderTarget* pRenderTarget) override;

	virtual void NextSubpass() override;

	virtual void BeginRenderPass(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget) override;

	virtual void EndRenderPass() override;

	virtual void RenderEnd(MIRenderTarget* pRenderTarget) override;
public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial) override;

	virtual bool SetRenderToTextureBarrier(const std::vector<MIRenderBackTexture*> vTextures) override;

	virtual bool DownloadTexture(MITexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) override;

	virtual bool CopyImageBuffer(MITexture* pSource, MITexture* pDest) override;

	virtual void UpdateMipmaps(MTextureBuffer* pBuffer) override;

public:

	void GetBlendStage(MMaterial* pMaterial, MRenderPass* pRenderPass, std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach, VkPipelineColorBlendStateCreateInfo& blendInfo);

	void GetDepthStencilStage(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo);

	void UpdateShaderParam(MShaderConstantParam& param, const uint32_t& unFrameIdx);

	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) override;

	VkPipeline CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx);

	bool InitSemaphores();
	void ReleaseSemaphores();
	

	VkCommandBuffer GetCommandBuffer();
private:

	MVulkanDevice* m_pDevice;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
	VkPipelineMultisampleStateCreateInfo m_MultisampleState;

	VkViewport m_VkViewport;


	struct MRenderStage
	{
		MRenderStage() :vkCommandBuffer(VK_NULL_HANDLE), pUsingMaterial(nullptr), pUsingPipelineLayoutData(nullptr) {}

		VkCommandBuffer vkCommandBuffer;
		struct MMaterial* pUsingMaterial;
		struct MMaterialPipelineLayoutData* pUsingPipelineLayoutData;
	};
	std::vector<MRenderStage> m_vRenderStages;


	struct MRenderPassStage
	{
		MRenderPassStage() : pRenderPass(nullptr), nSubpassIdx(0) {}
		MRenderPassStage(MRenderPass* p, const uint32_t& n) : pRenderPass(p), nSubpassIdx(n) {}

		MRenderPass* pRenderPass;
		uint32_t nSubpassIdx;
	};

	std::stack<MRenderPassStage> m_vRenderPassStages;

	//渲染栅栏，防止上一次渲染还没渲染完，CPU就申请执行下一次的渲染
	std::array<VkFence, M_BUFFER_NUM> m_VkInFlightFences;

	//渲染完成回调
	std::array<std::vector<std::function<void()> >, M_BUFFER_NUM> m_aRenderFinishedCallback;

private:

	uint32_t m_unFrameIndex;

};

#endif


#endif