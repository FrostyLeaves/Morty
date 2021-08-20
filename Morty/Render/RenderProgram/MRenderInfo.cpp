#include "MRenderInfo.h"



MRenderInfo::MRenderInfo()
	: nFrameIndex(0)
	, fDelta(0.0f)
	, fGameTime(0.0f)
	, pViewport(nullptr)
	, pCameraEntity(nullptr)
	, pDirectionalLightEntity(nullptr)
	, pShadowMapTexture(nullptr)
	, pFrameShaderParamSet(nullptr)
	, m4DirLightInvProj()
	, cShadowRenderAABB()
	, cMeshRenderAABB()
	, pPrimaryRenderCommand(nullptr)
{

}
