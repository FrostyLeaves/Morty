#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MSkyBoxSystem.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMaterialResource.h"

#include "Component/MSkyBoxComponent.h"
#include "Resource/MTextureResourceUtil.h"

//https://matheowis.github.io/HDRI-to-CubeMap/

void ENVIRONMENT_CUBEMAP_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MSkyBoxSystem* pSkyBoxSystem = pEngine->FindSystem<MSkyBoxSystem>();


	std::shared_ptr<MTextureResource> pCubeTexture = pResourceSystem->CreateResource<MTextureResource>();

	/*
	pCubeTexture->ImportCubeMap({
		"Texture/Sky/HDR_041/px.hdr",
		"Texture/Sky/HDR_041/nx.hdr",
		"Texture/Sky/HDR_041/py.hdr",
		"Texture/Sky/HDR_041/ny.hdr",
		"Texture/Sky/HDR_041/pz.hdr",
		"Texture/Sky/HDR_041/nz.hdr"
		}, { MTextureResource::MTexturePixelFormat::Float32 });
    */

	auto resourceData = MTextureResourceUtil::ImportCubeMap({
		pResourceSystem->GetFullPath("Texture/Sky/Circus_Backstage/px.hdr"),
		pResourceSystem->GetFullPath("Texture/Sky/Circus_Backstage/nx.hdr"),
		pResourceSystem->GetFullPath("Texture/Sky/Circus_Backstage/py.hdr"),
		pResourceSystem->GetFullPath("Texture/Sky/Circus_Backstage/ny.hdr"),
		pResourceSystem->GetFullPath("Texture/Sky/Circus_Backstage/pz.hdr"),
		pResourceSystem->GetFullPath("Texture/Sky/Circus_Backstage/nz.hdr")
		}, { MTexturePixelType::Float32 });

	pCubeTexture->Load(std::move(resourceData));


	MEntity* pSkyBoxEntity = pScene->CreateEntity();
	pSkyBoxEntity->SetName("SkyBox");

	if (MSkyBoxComponent* pSkyBoxComponent = pSkyBoxEntity->RegisterComponent<MSkyBoxComponent>())
	{
		pSkyBoxComponent->LoadSkyBoxResource(pCubeTexture);

		pSkyBoxSystem->GenerateEnvironmentTexture(pSkyBoxComponent);
	}
}
