#include "MRenderInfo.h"


#include "MScene.h"
#include "MViewport.h"
#include "MMaterial.h"

#include "MSceneComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

MRenderInfo::MRenderInfo()
	: nFrameIndex(0)
	, fDelta(0.0f)
	, fGameTime(0.0f)
	, pViewport(nullptr)
	, pCameraEntity(nullptr)
	, pDirectionalLightEntity(nullptr)
	, pSkyBoxEntity(nullptr)
	, pShadowMapTexture(nullptr)
	, m4DirLightInvProj()
	, cCaclSceneRenderAABB()
	, pPrimaryRenderCommand(nullptr)
{

}
