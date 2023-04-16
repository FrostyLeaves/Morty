#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MSkyBoxSystem.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMaterialResource.h"

#include "Component/MSkyBoxComponent.h"

//https://matheowis.github.io/HDRI-to-CubeMap/

void ENVIRONMENT_CUBEMAP_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MSkyBoxSystem* pSkyBoxSystem = pEngine->FindSystem<MSkyBoxSystem>();


	std::shared_ptr<MTextureResource> pCubeTexture = pResourceSystem->CreateResource<MTextureResource>();

	pCubeTexture->ImportCubeMap({
		"Texture/Sky/HDR_041/px.hdr",
		"Texture/Sky/HDR_041/nx.hdr",
		"Texture/Sky/HDR_041/py.hdr",
		"Texture/Sky/HDR_041/ny.hdr",
		"Texture/Sky/HDR_041/pz.hdr",
		"Texture/Sky/HDR_041/nz.hdr"
		}, { MTextureResource::PixelFormat::Float32 });


	MEntity* pSkyBoxEntity = pScene->CreateEntity();
	pSkyBoxEntity->SetName("SkyBox");

	if (MSkyBoxComponent* pSkyBoxComponent = pSkyBoxEntity->RegisterComponent<MSkyBoxComponent>())
	{
		pSkyBoxComponent->LoadSkyBoxResource(pCubeTexture);

		pSkyBoxSystem->GenerateEnvironmentTexture(pSkyBoxComponent);
	}
}
