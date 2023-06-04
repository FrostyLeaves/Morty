#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"

void CREATE_FLOOR(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->Load(MMeshResourceUtil::CreatePlane());

	MEntity* pFloorEntity = pScene->CreateEntity();
	pFloorEntity->SetName("Floor");

	MSceneComponent* pSceneComponent = pFloorEntity->RegisterComponent<MSceneComponent>();

	if (MRenderableMeshComponent* pMeshComponent = pFloorEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		pMeshComponent->SetSceneCullEnable(false);

		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

		pMaterial->LoadVertexShader("Shader/Floor/floor.mvs");
		pMaterial->LoadPixelShader("Shader/Floor/floor.mps");
		pMaterial->SetRasterizerType(MERasterizerType::ECullNone);
		pMaterial->SetMaterialType(MEMaterialType::ECustom);
		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}
