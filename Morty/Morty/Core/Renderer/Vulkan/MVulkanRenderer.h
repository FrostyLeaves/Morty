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
class MORTY_CLASS MVulkanRenderer : public MIRenderer
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

	virtual void BeginRenderPass(MIRenderTarget* pRenderTarget) override;

	virtual void EndRenderPass(MIRenderTarget* pRenderTarget) override;

	virtual void RenderEnd(MIRenderTarget* pRenderTarget) override;
public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial) override;

public:

	void GetBlendStage(MMaterial* pMaterial, MRenderPass* pRenderPass, std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach, VkPipelineColorBlendStateCreateInfo& blendInfo);

	void GetDepthStencilStage(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo);

	void UpdateShaderParam(MShaderConstantParam& param);

	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) override;

	VkPipeline CreateGraphicsPipeline(MMaterial* pMaaterial, MRenderPass* pRenderPass);

	void GetRenderTargetBarrier(MIRenderTarget* pRenderTarget, std::vector<VkImageMemoryBarrier>& vResult);;
	
	bool InitSemaphores();
	void ReleaseSemaphores();
	

	VkCommandBuffer GetCommandBuffer();
private:

	MVulkanDevice* m_pDevice;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo m_RasterizationState;
	VkPipelineMultisampleStateCreateInfo m_MultisampleState;

	VkViewport m_VkViewport;


	struct MRenderStage
	{
		MRenderStage() :vkCommandBuffer(VK_NULL_HANDLE), pUsingPipelineLayoutData(nullptr), vRenderTargetEvent() {}

		VkCommandBuffer vkCommandBuffer;
		struct MMaterialPipelineLayoutData* pUsingPipelineLayoutData;

		std::vector<VkEvent> vRenderTargetEvent;
	};
	std::vector<MRenderStage> m_vRenderStages;

	//渲染栅栏，防止上一次渲染还没渲染完，CPU就申请执行下一次的渲染
	std::array<VkFence, M_BUFFER_NUM> m_VkInFlightFences;
private:

	uint32_t m_unFrameIndex;
};

#endif


#endif