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
#include "MShadowTextureRenderTarget.h"
#include "Shader/MShaderParamSet.h"

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

		std::vector<MShadowRenderGroup> vShadowGroup;

	};

public:
	M_OBJECT(MForwardRenderProgram);
	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

    virtual void Render(MIRenderer* pRenderer, MIRenderTarget* pRenderTarget, const std::vector<MViewport*>& vViewports) override;

	void RenderWithViewport(MRenderInfo info, MViewport* pViewport);

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

protected:

	void GenerateRenderGroup(MRenderInfo& info);
	void UpdateShaderSharedParams(MRenderInfo& info);
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

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

private:

	MShaderParamSet m_FrameParamSet;


	MShaderConstantParam* m_pWorldMatrixParam;
	MShaderConstantParam* m_pWorldInfoParam;
	MShaderConstantParam* m_pLightInfoParam;

	MShaderSampleParam* m_pDefaultSampleParam;
	MShaderSampleParam* m_pLessEqualSampleParam;
	MShaderSampleParam* m_pGreaterEqualSampleParam;

	MShaderTextureParam* m_pShadowTextureParam;
	MShaderTextureParam* m_pTransparentFrontTextureParam;
	MShaderTextureParam* m_pTransparentBackTextureParam;

	MForwardShadowMapWork* m_pShadowMapWork;
	MForwardTransparentWork* m_pTransparentWork;

};

#endif
