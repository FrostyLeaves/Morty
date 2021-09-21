/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MDEFERRED_RENDERPROGRAM_H_
#define _M_MDEFERRED_RENDERPROGRAM_H_
#include "MGlobal.h"

#include "MType.h"
#include "MMesh.h"
#include "MBounds.h"
#include "MTaskNode.h"
#include "MTaskGraph.h"
#include "MRenderPass.h"
#include "MRenderInfo.h"
#include "MIRenderProgram.h"

#include "MShadowMapShaderParamSet.h"
#include "MForwardRenderShaderParamSet.h"

class MViewport;
class MMaterial;
class MIRenderCommand;
class MShadowMapRenderWork;
class MTransparentRenderWork;
class MRenderableMeshComponent;
class MORTY_API MDeferredRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MDeferredRenderProgram);

	MDeferredRenderProgram();
    virtual ~MDeferredRenderProgram();

public:

	void Render(MIRenderCommand* pPrimaryCommand);

	void RenderReady(MTaskNode* pTaskNode);

	void RenderGBuffer(MTaskNode* pTaskNode);

	void RenderLightning(MTaskNode* pTaskNode);

	void RenderShadow(MTaskNode* pTaskNode);

	void RenderTransparent(MTaskNode* pTaskNode);

	virtual MTexture* GetOutputTexture() override;


//debug

	MTexture* GetShadowmapTexture();

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void InitializeRenderPass();
	void ReleaseRenderPass();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();

protected:

	void DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand);

	void UpdateRenderGroup(MRenderInfo& info);

	void UpdateFrameParams(MRenderInfo& info);

	void ResizeForwardRenderPass(const Vector2& v2Size, MIDevice* pDevice);

protected:

	MTaskGraph* m_pRenderGraph;

	MRenderInfo m_renderInfo;

	MForwardRenderShaderParamSet m_frameParamSet;

	MRenderPass m_gbufferRenderPass;
	MRenderPass m_lightningRenderPass;

	MTexture* m_pFinalOutputTexture;

	MIRenderCommand* m_pPrimaryCommand;


	MIMesh* m_pScreenRectMesh;
	MMaterial* m_pLightningMaterial;

protected:

	MShadowMapRenderWork* m_pShadowMapWork;
	MTransparentRenderWork* m_pTransparentWork;

	size_t m_nFrameIndex;
};

#endif
