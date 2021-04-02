#ifndef M_RENDER_INFO_H_
#define M_RENDER_INFO_H_

#include "MGlobal.h"
#include "Vector.h"
#include "MBounds.h"
#include "MMaterialGroup.h"
#include "Shader/MShaderParamSet.h"

struct MRenderInfo
{
	MRenderInfo();

	float fDelta;
	float fGameTime;
	uint32_t unFrameIndex;
	class MIRenderTarget* pRenderTarget;
	class MIRenderer* pRenderer;
	class MViewport* pViewport;
	class MCamera* pCamera;
	class MScene* pScene;
	class MRenderCommand* pPrimaryCommand;

	class MDirectionalLight* pDirectionalLight;
	class MITexture* pShadowMapTexture;
	Matrix4 m4DirLightInvProj;

	MBoundsAABB cShadowRenderAABB;
	MBoundsAABB cMeshRenderAABB;

	std::vector<MMaterialGroup> vMaterialRenderGroup;
	std::vector<MMaterialGroup> vTransparentRenderGroup;

};



#endif