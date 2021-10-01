/**
 * @File         MForwardRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDRENDERPROGRAM_H_
#define _M_MFORWARDRENDERPROGRAM_H_
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
class MORTY_API MForwardRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MForwardRenderProgram);

	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

	virtual void Render(MIRenderCommand* pPrimaryCommand) override;

	void RenderReady(MTaskNode* pTaskNode);

	void RenderForward(MTaskNode* pTaskNode);

	void RenderShadow(MTaskNode* pTaskNode);

	void RenderTransparent(MTaskNode* pTaskNode);

	virtual MTexture* GetOutputTexture() override;
	virtual std::vector<MTexture*> GetOutputTextures() override;

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void InitializeRenderPass();
	void ReleaseRenderPass();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();

protected:

	void DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand);

	void UpdateFrameParams(MRenderInfo& info);

protected:

	MTaskGraph* m_pRenderGraph;

	MRenderInfo m_renderInfo;

	MForwardRenderShaderParamSet m_frameParamSet;

	MRenderPass m_forwardRenderPass;

	MTexture* m_pFinalOutputTexture;

	MIRenderCommand* m_pPrimaryCommand;

protected:

	MShadowMapRenderWork* m_pShadowMapWork;
	MTransparentRenderWork* m_pTransparentWork;

	size_t m_nFrameIndex;
};

#endif
