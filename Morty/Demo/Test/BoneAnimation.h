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


void BONE_ANIMATION_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MResource> pPigeonResource = pResourceSystem->LoadResource("D:/test_pigeon/pigeon/pigeon.entity");
	if (!pPigeonResource)
	{
		MModelConverter convert(pEngine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
		info.strOutputDir = "D:/test_pigeon";
		info.strOutputName = "pigeon";
		info.strResourcePath = "./Model/pigeon/source/Pigeon_Animations.fbx";

		convert.Convert(info);

		pPigeonResource = pResourceSystem->LoadResource("D:/test_pigeon/pigeon/pigeon.entity");
	}

	std::vector<MComponentID> vMeshComponents;
	for (size_t i = 0; i < 1; ++i)
	{
		auto vEntity = pEntitySystem->LoadEntity(pScene, pPigeonResource);

		for (MEntity* pEntity : vEntity)
		{
			pEntitySystem->FindAllComponentRecursively(pEntity, MRenderMeshComponent::GetClassType(), vMeshComponents);

		}
	}

	for (MComponentID& componentID : vMeshComponents)
	{
		if (MRenderMeshComponent* pMeshComponent = pScene->GetComponent(componentID)->template DynamicCast<MRenderMeshComponent>())
		{
			pMeshComponent->SetGenerateDirLightShadow(true);
		}
	}
}
