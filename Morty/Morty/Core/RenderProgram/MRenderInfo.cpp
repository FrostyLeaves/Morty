#include "MRenderInfo.h"



MRenderInfo::MRenderInfo()
	: fDelta(0.0f)
	, fGameTime(0.0f)
	, unFrameIndex(0)
	, pRenderTarget(nullptr)
	, pRenderer(nullptr)
	, pViewport(nullptr)
	, pCamera(nullptr)
	, pScene(nullptr)
	, pDirectionalLight(nullptr)
	, pShadowMapTexture(nullptr)
	, pPrimaryCommand(nullptr)
	, m4DirLightInvProj()
	, cShadowRenderAABB()
	, cMeshRenderAABB()
	, vMaterialRenderGroup()
	, vTransparentRenderGroup()
{

}
