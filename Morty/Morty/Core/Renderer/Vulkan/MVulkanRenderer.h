/**
 * @File         MVulkanRenderer
 * 
 * @Created      2020-06-18 15:18:31
 *
 * @Author       DoubleYe
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

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetViewport(MRenderCommand* pCommand, const MViewportInfo& viewport) override;

	virtual void SetScissor(MRenderCommand* pCommand, const MScissorInfo& scissor) override;

	virtual void RenderCommandBegin(MRenderCommand* pCommand) override;
	virtual void RenderCommandEnd(MRenderCommand* pCommand) override;
	virtual void SubmitRenderCommand(MRenderCommand* pCommand, MIRenderTarget* pRenderTarget) override;

	virtual void NextSubpass(MRenderCommand* pCommand) override;

	virtual void BeginRenderPass(MRenderCommand* pCommand, MRenderPass* pRenderPass, const uint32_t& nFrameBufferIdx) override;

	virtual void EndRenderPass(MRenderCommand* pCommand) override;

public:
	virtual void DrawMesh(MRenderCommand* pCommand, MIMesh* pMesh) override;

	virtual void DrawMesh(MRenderCommand* pCommand, MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset) override;

	virtual bool SetUseMaterial(MRenderCommand* pCommand, MMaterial* pMaterial) override;

	virtual bool SetRenderToTextureBarrier(MRenderCommand* pCommand, const std::vector<MIRenderTexture*> vTextures) override;

	virtual bool DownloadTexture(MRenderCommand* pCommand, MITexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) override;

	virtual bool CopyImageBuffer(MRenderCommand* pCommand, MITexture* pSource, MITexture* pDest) override;

	virtual void UpdateMipmaps(MRenderCommand* pCommand, MTextureBuffer* pBuffer) override;

public:

	void GetBlendStage(MMaterial* pMaterial, MRenderPass* pRenderPass, std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach, VkPipelineColorBlendStateCreateInfo& blendInfo);

	void GetDepthStencilStage(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo);

	void UpdateShaderParam(MShaderConstantParam& param, const uint32_t& unFrameIdx);

	virtual void SetShaderParamSet(MRenderCommand* pCommand, MShaderParamSet* pParamSet) override;

	VkPipeline CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx);

	
private:

	MVulkanDevice* m_pDevice;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
	VkPipelineMultisampleStateCreateInfo m_MultisampleState;



};

#endif


#endif