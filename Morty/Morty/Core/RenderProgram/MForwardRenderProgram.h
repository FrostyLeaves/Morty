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
class MRenderGraph;
class MIMeshInstance;
class MModelInstance;
class MSkeletonInstance;
class MDirectionalLight;
class MForwardRenderWork;
class MRenderTexture;
class MIModelMeshInstance;
class MForwardShadowMapWork;
class MForwardTransparentWork;
class MShadowTextureRenderTarget;

struct MRenderInfo
{
	MRenderInfo();

	float fDelta;
	uint32_t unFrameIndex;
	class MIRenderTarget* pRenderTarget;
	class MIRenderer* pRenderer;
	class MViewport* pViewport;
	class MCamera* pCamera;
	class MScene* pScene;

	class MDirectionalLight* pDirectionalLight;
	class MITexture* pShadowMapTexture;
	Matrix4 m4DirLightInvProj;

	MBoundsAABB cShadowRenderAABB;
	MBoundsAABB cMeshRenderAABB;

	std::vector<MMaterialGroup> vMaterialRenderGroup;
	std::vector<MMaterialGroup> vTransparentRenderGroup;

};

class MORTY_API MForwardRenderProgram : public MIRenderProgram
{
public:
	M_OBJECT(MForwardRenderProgram);
	MForwardRenderProgram();
    virtual ~MForwardRenderProgram();

public:

    virtual void Render(MIRenderer* pRenderer, MViewport* pViewport) override;

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	virtual void SetClearColor(const MColor& cClearColor) override;

	virtual MRenderGraph* GetRenderGraph() override { return m_pRenderGraph; }
public:

	void Render(MRenderInfo& info);

	void GenerateRenderGroup(MRenderInfo& info);

protected:

	virtual void Initialize() override;
	virtual void Release() override;

protected:

	MForwardShadowMapWork* m_pShadowMapWork;
	MForwardRenderWork* m_pRenderWork;
	MForwardTransparentWork* m_pTransparentWork;

	MColor m_cClearColor;


	MRenderGraph* m_pRenderGraph;
};

#endif
