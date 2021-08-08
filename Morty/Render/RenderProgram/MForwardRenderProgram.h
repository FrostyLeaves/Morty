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
#include "MForwardRenderShaderParamSet.h"

class MViewport;
class MMaterial;
class MIRenderCommand;
class MRenderableMeshComponent;
class MORTY_API MForwardRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MForwardRenderProgram);

	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

	void Render();

	void RenderReady(MTaskNode* pTaskNode);

	void RenderShadow(MTaskNode* pTaskNode);

	void RenderForward(MTaskNode* pTaskNode);

	virtual MTexture* GetOutputTexture() override { return m_pOutputTexture; }

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;


protected:

	void DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand);

	void GenerateRenderGroup(MRenderInfo& info);

	void UpdateFrameParams(MRenderInfo& info);

protected:

	MTaskGraph* m_pRenderGraph;

	MRenderInfo m_renderInfo;
	MForwardRenderShaderParamSet m_frameParamSet;


	MTexture* m_pShadowMapTexture;
	MTexture* m_pOutputTexture;
	MTexture* m_pDepthTexture;
};

#endif
