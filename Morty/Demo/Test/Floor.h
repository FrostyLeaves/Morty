#include <Resource/MMeshResourceUtil.h>
#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"

void CREATE_FLOOR_GRID(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->Load(MMeshResourceUtil::CreatePlane());

	MEntity* pFloorEntity = pScene->CreateEntity();
	pFloorEntity->SetName("Floor Grid");

	if (MRenderMeshComponent* pMeshComponent = pFloorEntity->RegisterComponent<MRenderMeshComponent>())
	{
		pMeshComponent->SetSceneCullEnable(false);


		const auto pTemplate = pResourceSystem->CreateResource<MMaterialTemplate>("Floor Material");
		pTemplate->LoadShader("Shader/Floor/floor.mvs");
		pTemplate->LoadShader("Shader/Floor/floor.mps");
		pTemplate->SetCullMode(MECullMode::ECullNone);
		pTemplate->SetMaterialType(MEMaterialType::ECustom);
		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(MMaterialResource::CreateMaterial(pTemplate));
	}
}
