/**
 * @File         MForwardRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDRENDERPROGRAM_H_
#define _M_MFORWARDRENDERPROGRAM_H_
#include "MGlobal.h"
#include "MMesh.h"
#include "Vector.h"
#include "MBounds.h"
#include "MIRenderProgram.h"
#include "MMaterialGroup.h"
#include "Shader/MShaderParamSet.h"

#include "MRenderPass.h"
#include "MForwardRenderShaderParamSet.h"

#include <vector>

class MScene;
class MCamera;
class MIMeshInstance;
class MModelInstance;
class MSkeletonInstance;
class MDirectionalLight;
class MRenderBackTexture;
class MIModelMeshInstance;
class MRenderDepthTexture;
class MForwardShadowMapWork;
class MForwardTransparentWork;
class MShadowTextureRenderTarget;
class MORTY_CLASS MForwardRenderProgram : public MIRenderProgram
{
public:

	struct MRenderInfo
	{
		uint32_t unFrameIndex;
		MIRenderTarget* pRenderTarget;
		MIRenderer* pRenderer;
		MViewport* pViewport;
		MCamera* pCamera;
		MScene* pScene;

		MDirectionalLight* pDirectionalLight;
		Matrix4 m4DirLightInvProj;

		MBoundsAABB cShadowRenderAABB;
		MBoundsAABB cMeshRenderAABB;

 		std::vector<MMaterialGroup> vMaterialRenderGroup;
		std::vector<MMaterialGroup> vTransparentRenderGroup;

	};

public:
	M_OBJECT(MForwardRenderProgram);
	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

    virtual void Render(MIRenderer* pRenderer, const std::vector<MViewport*>& vViewports) override;

	void RenderWithViewport(MRenderInfo info, MViewport* pViewport);

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	static void UpdateShaderSharedParams(MRenderInfo& info, MForwardRenderShaderParamSet& frameParamSet);

protected:

	void GenerateRenderGroup(MRenderInfo& info);
	void DrawNormalMesh(MRenderInfo& info);

	void DrawModelInstance(MRenderInfo& info);
	void DrawSkyBox(MRenderInfo& info);
	void DrawPainter(MRenderInfo& info);
	void DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns);
	void DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pModelIns);
	void DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera);


public:

	virtual void DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance) override;

protected:

	virtual void Initialize() override;
	virtual void Release() override;

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

	void InitializeRenderPass();
	void ReleaseRenderPass();

private:

	MForwardRenderShaderParamSet m_FrameParamSet;

	MForwardShadowMapWork* m_pShadowMapWork;
	MForwardTransparentWork* m_pTransparentWork;

	MRenderPass m_ForwardMeshRenderPass;
};

#endif
