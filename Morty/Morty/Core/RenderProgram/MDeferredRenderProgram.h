/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MDEFERRED_RDRENDERPROGRAM_H_
#define _M_MDEFERRED_RDRENDERPROGRAM_H_
#include "MGlobal.h"
#include "MIRenderProgram.h"
#include "Shader/MShaderParamSet.h"

#include "MRenderInfo.h"
#include "MRenderPass.h"
#include "MRenderGraph.h"
#include "MForwardRenderShaderParamSet.h"

#include <vector>

class MScene;
class MCamera;
class MRenderGraph;
class MIMeshInstance;
class MModelInstance;
class MSkeletonInstance;
class MDirectionalLight;
class MDeferredRenderWork;
class MRenderTexture;
class MRenderCommand;
class MIModelMeshInstance;
class MForwardShadowMapWork;
class MForwardTransparentWork;
class MForwardDebugRenderWork;
class MShadowTextureRenderTarget;

class MORTY_API MDeferredRenderProgram : public MIRenderProgram
{
public:
	M_OBJECT(MDeferredRenderProgram);
	MDeferredRenderProgram();
    virtual ~MDeferredRenderProgram();

public:

	//TODO Secondary Command.
    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport, MRenderCommand* pCommand) override;

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	virtual void SetClearColor(const MColor& cClearColor) override;

	virtual MRenderGraph* GetRenderGraph() override { return m_pRenderGraph; }
public:

	void Render(MRenderInfo& info);

	void GenerateRenderGroup(MRenderInfo& info);

	virtual MRenderInfo* GetRenderInfo() override { return &m_RenderInfo; }

protected:

	virtual void Initialize() override;
	virtual void Release() override;

protected:

	MRenderInfo m_RenderInfo;

	MForwardShadowMapWork* m_pShadowMapWork;
	MDeferredRenderWork* m_pRenderWork;
	MForwardTransparentWork* m_pTransparentWork;
	MForwardDebugRenderWork* m_pDebugWork;

	MColor m_cClearColor;


	MRenderGraph* m_pRenderGraph;
};

#endif
