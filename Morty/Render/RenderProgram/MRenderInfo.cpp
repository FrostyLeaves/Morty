#include "MRenderInfo.h"


#include "Scene/MScene.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

MRenderInfo::MRenderInfo()
	: nFrameIndex(0)
	, fDelta(0.0f)
	, fGameTime(0.0f)
	, pViewport(nullptr)
	, pCameraEntity(nullptr)
	, pDirectionalLightEntity(nullptr)
	, pSkyBoxEntity(nullptr)
	, pPrimaryRenderCommand(nullptr)
{

}
