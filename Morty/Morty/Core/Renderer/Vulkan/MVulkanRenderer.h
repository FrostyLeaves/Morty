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

#include "MIRenderer.h"

class MRenderPass;
class MVulkanDevice;
class MORTY_CLASS MVulkanRenderer : public MIRenderer
{
public:
    MVulkanRenderer(MVulkanDevice* pDevice);
    virtual ~MVulkanRenderer();

public:

	uint32_t GetFrameIndex() { return m_unFrameIndex; }
	void SetFrameIndex(const uint32_t& unIndex) { m_unFrameIndex = unIndex; }

	virtual void AddOutputView(MIRenderView* pView) override;

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) override;
	
	virtual void Render(MIRenderTarget* pRenderTarget) override;

public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources = false) override;
	virtual void UpdateMaterialParam() override {}
	virtual void UpdateMaterialResource() override {}

public:
	void UpdateShaderParam(MShaderParam& param);

	virtual void SetShaderParam(MShaderParam& param) override;

	virtual void SetVertexShaderTexture(MShaderTextureParam& param) override {}
	virtual void SetPixelShaderTexture(MShaderTextureParam& param) override {}

protected:
	virtual void ClearRenderTargetView(MRenderTextureBuffer* pRenderTextureBuffer, const MColor& color) override;
	virtual void ClearDepthTexture(MRenderDepthTexture* pDepthTexture) override {};

	VkPipeline CreateGraphicsPipeline(MMaterial* pMaaterial, MRenderPass* pRenderPass);
	
	bool InitSemaphores();


public:
	VkSemaphore m_VkImageAvailableSemaphore;
	VkSemaphore m_VkRenderFinishedSemaphore;
	VkFence m_VkInFlightFences;

private:


	MVulkanDevice* m_pDevice;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo m_RasterizationState;
	VkPipelineMultisampleStateCreateInfo m_MultisampleState;
	VkPipelineColorBlendAttachmentState m_ColorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo m_ColorBlending;

	VkViewport m_VkViewport;

	VkCommandBuffer m_VkCommandBuffer;
	VkPipeline m_VkUsingPipeline;
	VkPipelineLayout m_VkUsingPipelineLayout;

	MMaterial* m_pUsingMaterial;

private:


	uint32_t m_unFrameIndex;
};


#endif


#endif