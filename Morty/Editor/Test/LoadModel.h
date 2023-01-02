#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"
#include "Widget/ModelConvertView.h"


void LOAD_MODEL_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MResource> pModelResource = pResourceSystem->LoadResource("./LilacSoup/LilacSoup/LilacSoup.entity");
	if (!pModelResource)
	{
		MModelConverter convert(pEngine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
		info.strOutputDir = "./LilacSoup";
		info.strOutputName = "LilacSoup";
		info.strResourcePath = "./Model/LilacSoup/LilacPOSE.obj";

		convert.Convert(info);

		pModelResource = pResourceSystem->LoadResource("./LilacSoup/LilacSoup/LilacSoup.entity");
	}

	std::vector<MComponentID> vMeshComponents;
	for (size_t i = 0; i < 1; ++i)
	{
		auto&& vEntity = pEntitySystem->LoadEntity(pScene, pModelResource);

		for (MEntity* pEntity : vEntity)
		{
			pEntitySystem->FindAllComponentRecursively(pEntity, MRenderableMeshComponent::GetClassType(), vMeshComponents);
		}
	}

	for (MComponentID& componentID : vMeshComponents)
	{
		if (MRenderableMeshComponent* pMeshComponent = pScene->GetComponent(componentID)->DynamicCast<MRenderableMeshComponent>())
		{
			pMeshComponent->SetGenerateDirLightShadow(true);
		}
	}
}
