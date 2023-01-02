#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MSkyBoxSystem.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMaterialResource.h"

#include "Component/MSkyBoxComponent.h"


void ENVIRONMENT_CUBEMAP_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MSkyBoxSystem* pSkyBoxSystem = pEngine->FindSystem<MSkyBoxSystem>();


	std::shared_ptr<MTextureResource> pCubeTexture = pResourceSystem->CreateResource<MTextureResource>();

	pCubeTexture->ImportCubeMap({
		"Texture/Sky/Circus_Backstage/px.hdr",
		"Texture/Sky/Circus_Backstage/nx.hdr",
		"Texture/Sky/Circus_Backstage/py.hdr",
		"Texture/Sky/Circus_Backstage/ny.hdr",
		"Texture/Sky/Circus_Backstage/pz.hdr",
		"Texture/Sky/Circus_Backstage/nz.hdr"
		}, { MTextureResource::PixelFormat::Float32 });


	MEntity* pSkyBoxEntity = pScene->CreateEntity();
	pSkyBoxEntity->SetName("SkyBox");

	if (MSkyBoxComponent* pSkyBoxComponent = pSkyBoxEntity->RegisterComponent<MSkyBoxComponent>())
	{
		pSkyBoxComponent->LoadSkyBoxResource(pCubeTexture);

		pSkyBoxSystem->GenerateEnvironmentTexture(pSkyBoxComponent);
	}
}
