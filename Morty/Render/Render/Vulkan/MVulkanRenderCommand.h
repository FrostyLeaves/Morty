/**
 * @File         MVulkanRenderCommand
 * 
 * @Created      2021-07-14 18:22:21
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANRENDERCOMMAND_H_
#define _M_MVULKANRENDERCOMMAND_H_
#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Render/Vulkan/MVulkanDevice.h"
#include "Render/MRenderCommand.h"

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

	virtual bool SetUseMaterial(std::shared_ptr<MMaterial> pMaterial) override;
	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) override;

	virtual bool DispatchComputeJob(std::shared_ptr<MComputeDispatcher> pMaterial) override;

	virtual bool SetRenderToTextureBarrier(const std::vector<MTexture*> vTextures) override;
	virtual bool DownloadTexture(MTexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback) override;
	virtual bool CopyImageBuffer(MTexture* pSource, MTexture* pDest) override;
	virtual void UpdateMipmaps(MTexture* pBuffer) override;

	virtual void addFinishedCallback(std::function<void()> func) override;

	void UpdateBuffer(MBuffer* pBuffer);

	void UpdateShaderParam(MShaderParamSet* pParamSet, MShaderConstantParam* param);
	void UpdateShaderParam(MShaderParamSet* pParamSet, MShaderTextureParam* param);


	void BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam);
	void BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam);

	void SetTextureLayout(const std::vector<MTexture*>& vTextures, VkImageLayout newLayout);


public:

	MVulkanDevice* m_pDevice;

	std::shared_ptr<MMaterial> pUsingMaterial;
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
