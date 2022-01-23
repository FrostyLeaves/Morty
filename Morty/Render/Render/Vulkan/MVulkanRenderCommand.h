/**
 * @File         MVulkanRenderCommand
 * 
 * @Created      2021-07-14 18:22:21
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANRENDERCOMMAND_H_
#define _M_MVULKANRENDERCOMMAND_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MVulkanDevice.h"
#include "MRenderCommand.h"

class MTexture;
class MORTY_API MVulkanRenderCommand : public MIRenderCommand
{
public:
    MVulkanRenderCommand();
    virtual ~MVulkanRenderCommand();

public:

	virtual void SetViewport(const MViewportInfo& viewport) override;
	virtual void SetScissor(const MScissorInfo& scissor) override;

	virtual void RenderCommandBegin() override;
	virtual void RenderCommandEnd() override;

	virtual void BeginRenderPass(MRenderPass* pRenderPass) override;
	virtual void NextSubpass() override;
	virtual void EndRenderPass() override;

	virtual void DrawMesh(MIMesh* pMesh) override;
	virtual void DrawMesh(MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial) override;
	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) override;

	virtual bool SetRenderToTextureBarrier(const std::vector<MTexture*> vTextures) override;
	virtual bool DownloadTexture(MTexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) override;
	virtual bool CopyImageBuffer(MTexture* pSource, MTexture* pDest) override;
	virtual void UpdateMipmaps(MTexture* pBuffer) override;

	virtual void addFinishedCallback(std::function<void()> func) override;

	void UpdateShaderParam(MShaderParamSet* pParamSet, MShaderConstantParam* param);
	void UpdateShaderParam(MShaderParamSet* pParamSet, MShaderTextureParam* param);

	VkPipeline CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx);

	void BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam);
	void BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam);

	void SetTextureLayout(const std::vector<MTexture*>& vTextures, VkImageLayout newLayout);


public:

	MVulkanDevice* m_pDevice;

	MMaterial* pUsingMaterial;
	MMaterialPipelineLayoutData* pUsingPipelineLayoutData;
	std::stack<MRenderPassStage> m_vRenderPassStages;

	VkCommandBuffer m_VkCommandBuffer;

	std::map<MTexture*, VkImageLayout> m_tTextureLayout;

	std::vector<std::function<void()>> m_aRenderFinishedCallback;
};

class MORTY_API MVulkanSecondaryRenderCommand : public MVulkanRenderCommand
{
public:

};

class MORTY_API MVulkanPrimaryRenderCommand : public MVulkanRenderCommand
{
public:
	MVulkanPrimaryRenderCommand();

	virtual bool IsFinished() override { return m_bFinished; }
	virtual void CheckFinished() override;

	virtual MIRenderCommand* CreateChildCommand() override;
	virtual MIRenderCommand* GetChildCommand(const size_t& nIndex) override;
	virtual void ExecuteChildCommand() override;

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

#endif
