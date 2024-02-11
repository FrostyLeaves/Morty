#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"
#include "Widget/ModelConvertView.h"

using namespace morty;

void LOAD_MODEL_ANIMATION_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MResource> pModelResource = pResourceSystem->LoadResource("./pigeon/pigeon/pigeon.entity");
	if (!pModelResource)
	{
		MModelConverter convert(pEngine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
		info.strOutputDir = "./pigeon";
		info.strOutputName = "pigeon";
		info.strResourcePath = "./Model/pigeon/source/Pigeon_Animations.fbx";

		convert.Convert(info);

		pModelResource = pResourceSystem->LoadResource("./pigeon/pigeon/pigeon.entity");
	}

	std::vector<MComponentID> vMeshComponents;
	for (size_t i = 0; i < 100; ++i)
	{
		auto vEntity = pEntitySystem->LoadEntity(pScene, pModelResource);

		for (MEntity* pEntity : vEntity)
		{
			pEntity->GetComponent<MSceneComponent>()->SetScale(Vector3(1, 1, 1));
			pEntitySystem->FindAllComponentRecursively(pEntity, MRenderMeshComponent::GetClassType(), vMeshComponents);
		}

		vEntity[0]->GetComponent<MSceneComponent>()->SetPosition(Vector3(i / 10, 0, i % 10) * 5);
	}

	for (MComponentID& componentID : vMeshComponents)
	{
		if (MRenderMeshComponent* pMeshComponent = pScene->GetComponent(componentID)->template DynamicCast<MRenderMeshComponent>())
		{
			pMeshComponent->SetGenerateDirLightShadow(true);
		}
	}
}

void LOAD_MODEL_TRANSLATION_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MResource> pModelResource = pResourceSystem->LoadResource("./TestTranslation/TestTranslation/TestTranslation.entity");
	if (!pModelResource)
	{
		MModelConverter convert(pEngine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
		info.strOutputDir = "./TestTranslation";
		info.strOutputName = "TestTranslation";
		info.strResourcePath = "./Model/test_translation.gltf";

		convert.Convert(info);

		pModelResource = pResourceSystem->LoadResource("./TestTranslation/TestTranslation/TestTranslation.entity");
	}

	auto vEntity = pEntitySystem->LoadEntity(pScene, pModelResource);
}
