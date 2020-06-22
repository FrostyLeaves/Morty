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

class MVulkanDevice;
class MORTY_CLASS MVulkanRenderer : public MIRenderer
{
public:
    MVulkanRenderer(MVulkanDevice* pDevice);
    virtual ~MVulkanRenderer();


	virtual void AddOutputView(MIRenderView* pView) override;

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) override;
	virtual void RecoverRenderTarget(RenderTargetPair& pRenderTarget) override;
	virtual void ClearRenderTarget(MIRenderTarget* pRenderTarget) override;
public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources = false) override;
	virtual void UpdateMaterialParam() override;
	virtual void UpdateMaterialResource() override;

public:
	void UpdateShaderParam(MShaderParam& param);

	virtual void SetVertexShaderParam(MShaderParam& param) override;
	virtual void SetPixelShaderParam(MShaderParam& param) override;

	virtual void SetVertexShaderTexture(MShaderTextureParam& param) override;
	virtual void SetPixelShaderTexture(MShaderTextureParam& param) override;

protected:

	bool CreateGraphicsPipeline(MMaterial* pMaaterial);

	bool CreateRenderPass();

private:

	MVulkanDevice* m_pDevice;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo m_RasterizationState;
	VkPipelineViewportStateCreateInfo m_ViewportState;
	VkPipelineMultisampleStateCreateInfo m_MultisampleState;

};


#endif


#endif