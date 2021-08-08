#include "MRenderInfo.h"



MRenderInfo::MRenderInfo()
	: fDelta(0.0f)
	, fGameTime(0.0f)
	, pViewport(nullptr)
	, pCameraEntity(nullptr)
	, pDirectionalLightEntity(nullptr)
	, pShadowMapTexture(nullptr)
	, m4DirLightInvProj()
	, cShadowRenderAABB()
	, cMeshRenderAABB()
{

}
