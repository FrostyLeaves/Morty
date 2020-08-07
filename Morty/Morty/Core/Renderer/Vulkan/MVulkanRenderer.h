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

	uint32_t GetFrameIndex() { return m_unFrameIndex; }


	virtual void NewRenderFrame() override;

	virtual void AddOutputView(MIRenderView* pView) override;

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) override;
	
	virtual void Render(MIRenderTarget* pRenderTarget) override;

public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial) override;

public:
	void UpdateShaderParam(MShaderConstantParam& param);

	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) override;

	VkPipeline CreateGraphicsPipeline(MMaterial* pMaaterial, MRenderPass* pRenderPass);
	
	bool InitSemaphores();
	void ReleaseSemaphores();
	

public:
	VkSemaphore m_VkImageAvailableSemaphore;

private:

	MVulkanDevice* m_pDevice;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo m_RasterizationState;
	VkPipelineMultisampleStateCreateInfo m_MultisampleState;
	VkPipelineColorBlendAttachmentState m_ColorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo m_ColorBlending;
	VkPipelineDepthStencilStateCreateInfo m_DepthStencilState;

	VkViewport m_VkViewport;

	VkCommandBuffer m_VkCommandBuffer;
	struct MMaterialPipelineLayoutData* m_pUsingPipelineLayoutData;

	std::array<VkFence, M_BUFFER_NUM> m_VkInFlightFences;
private:

	uint32_t m_unFrameIndex;
};

#endif


#endif