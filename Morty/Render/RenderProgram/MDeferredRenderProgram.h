/**
 * @File         MDeferredRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MDEFERRED_RENDERPROGRAM_H_
#define _M_MDEFERRED_RENDERPROGRAM_H_
#include "Utility/MGlobal.h"

#include "Type/MType.h"
#include "Render/MMesh.h"
#include "Utility/MBounds.h"
#include "Resource/MResource.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskGraph.h"
#include "Render/MRenderPass.h"
#include "MRenderInfo.h"
#include "RenderProgram/MIRenderProgram.h"

#include "MShadowMapShaderParamSet.h"
#include "MForwardRenderShaderParamSet.h"

class MGPUCullingRenderWork;
class MViewport;
class MMaterial;
class MIRenderCommand;
class MComputeDispatcher;
class MShadowMapRenderWork;
class MTransparentRenderWork;
class MEnvironmentMapRenderWork;
class MRenderableMeshComponent;
class MORTY_API MDeferredRenderProgram : public MIRenderProgram
{
public:
	MORTY_CLASS(MDeferredRenderProgram);

	MDeferredRenderProgram();
    virtual ~MDeferredRenderProgram();

public:

	virtual void Render(MIRenderCommand* pPrimaryCommand) override;

	void RenderReady(MTaskNode* pTaskNode);

	void RenderCulling(MTaskNode* pTaskNode);

	void RenderGBuffer(MTaskNode* pTaskNode);

	void RenderLightning(MTaskNode* pTaskNode);

	void RenderShadow(MTaskNode* pTaskNode);

	void RenderForward(MTaskNode* pTaskNode);

	void RenderTransparent(MTaskNode* pTaskNode);

	void RenderDebug(MTaskNode* pTaskNode);



	virtual MTexture* GetOutputTexture() override;
	virtual std::vector<MTexture*> GetOutputTextures() override;

//debug

	MTexture* GetShadowmapTexture();

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void InitializeRenderPass();
	void ReleaseRenderPass();

	void InitializeFrameShaderParams();
	void ReleaseFrameShaderParams();

	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeMesh();
	void ReleaseMesh();

protected:

	void DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand, std::map<std::shared_ptr<MMaterial>, std::vector<MRenderableMeshComponent*>>& tMaterialGroup);

	void UpdateFrameParams(MRenderInfo& info);

	void CollectRenderMesh(MRenderInfo& info);

protected:

	MTaskGraph* m_pRenderGraph;

	MRenderInfo m_renderInfo;


	MRenderPass m_forwardRenderPass;
	MRenderPass m_gbufferRenderPass;
	MRenderPass m_lightningRenderPass;
	MRenderPass m_debugRenderPass;

	MTexture* m_pFinalOutputTexture;

	MIRenderCommand* m_pPrimaryCommand;


	MMesh<Vector2> m_ScreenDrawMesh;
	MMesh<Vector3> m_SkyBoxDrawMesh;
	std::shared_ptr<MMaterial> m_pLightningMaterial;
	std::shared_ptr<MMaterial> m_pSkyBoxMaterial;

	std::shared_ptr<MMaterial> m_pForwardMaterial;
	MForwardRenderShaderPropertyBlock m_forwardFramePropertyBlock;


	MResourceKeeper m_BrdfTexture;

protected:

	MShadowMapRenderWork* m_pShadowMapWork;
	MTransparentRenderWork* m_pTransparentWork;
	MGPUCullingRenderWork* m_pGPUCullingRenderWork;
	bool m_bGPUCullingUpdate = true;

	size_t m_nFrameIndex;
};

#endif
