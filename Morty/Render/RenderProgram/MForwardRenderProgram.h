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
class MRenderableMeshComponent;
class MORTY_API MForwardRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MForwardRenderProgram);

	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

	void Render(MIRenderCommand* pPrimaryCommand);

	void RenderReady(MTaskNode* pTaskNode);

	void RenderForward(MTaskNode* pTaskNode);

	void RenderShadow(MTaskNode* pTaskNode);

	virtual MTexture* GetOutputTexture() override;

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void InitializeRenderPass();
	void ReleaseRenderPass();
protected:

	void DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand);

	void UpdateRenderGroup(MRenderInfo& info);

	void UpdateFrameParams(MRenderInfo& info);

	void ResizeForwardRenderPass(const Vector2& v2Size, MIDevice* pDevice);

	void SubmitCommand(MTaskNode* pTaskNode);


protected:

	MTaskGraph* m_pRenderGraph;

	MRenderInfo m_renderInfo;

	MForwardRenderShaderParamSet m_frameParamSet;

	MTexture* m_pFinalOutputTexture;

	MRenderPass m_forwardRenderPass;

	MIRenderCommand* m_pPrimaryCommand;

protected:

	MShadowMapRenderWork* m_pShadowMapWork;

	size_t m_nFrameIndex;
};

#endif
