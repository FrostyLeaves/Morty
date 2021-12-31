// Demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "SDL.h"
#include <fstream>

#include "MEngine.h"
#include "MainEditor.h"

#include "MRenderModule.h"
#include "MModelConverter.h"

#include "MEditorModule.h"

#include "MEntitySystem.h"
#include "MResourceSystem.h"

#include "MMaterial.h"

#include "MScene.h"
#include "MSceneComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

#include "MMaterialResource.h"

#ifdef MORTY_WIN
#undef main
#endif

//#define TEST_ANIMATION

#define TEST_PBR_RENDER

void SPHERE_GENERATE(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	MMeshResource* pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->LoadAsSphere();

	MEntity* pCubeEntity = pScene->CreateEntity();
	pCubeEntity->SetName("Cube");
	if (MSceneComponent* pSceneComponent = pCubeEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pCubeEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		MMaterialResource* pMaterial = pResourceSystem->CreateResource<MMaterialResource>();


		pMaterial->LoadVertexShader("./Shader/model.mvs");
		pMaterial->LoadPixelShader("./Shader/model.mps");

		pMaterial->GetMaterialParamSet()->SetValue("f3Ambient", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Diffuse", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Specular", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("fAlphaFactor", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fShininess", 32.0f);

		MResource* pTextureResource = pResourceSystem->LoadResource("./Texture/test.jpg");
		pMaterial->SetTexutreParam("U_mat_texDiffuse", pTextureResource);

		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}

void PBR_MODEL(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();


	MResource* pDroneResource = pResourceSystem->LoadResource("D:/test_drone/drone/drone.entity");
	if (!pDroneResource)
	{
		MModelConverter convert(pEngine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;
		info.strOutputDir = "D:/test_drone";
		info.strOutputName = "drone";
		info.strResourcePath = "./Model/pbr/drone/Drone_Sketchfab.fbx";

		convert.Convert(info);

		pDroneResource = pResourceSystem->LoadResource("D:/test_drone/drone/drone.entity");
	}



	auto&& vEntity = pEntitySystem->LoadEntity(pScene, pDroneResource);

}

void ANIMATION_MODEL(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	MResource* pPigeonResource = pResourceSystem->LoadResource("D:/test_pigeon/pigeon/pigeon.entity");
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
		auto&& vEntity = pEntitySystem->LoadEntity(pScene, pPigeonResource);

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



int main()
{
	//initialize
	MEngine engine;
	engine.Initialize();

	//register module
	MRenderModule::Register(&engine);
	MEditorModule::Register(&engine);


	bool bClosed = false;
	
	MainEditor editor;
	editor.Initialize(&engine, "Morty");
	editor.SetCloseCallback([&bClosed]() {

		bClosed = true;
		return true;
		});

	if (MScene* pScene = editor.GetScene())
	{
		MResourceSystem* pResourceSystem = engine.FindSystem<MResourceSystem>();
		MEntitySystem* pEntitySystem = engine.FindSystem<MEntitySystem>();

		
#ifdef	TEST_ANIMATION
		ANIMATION_MODEL(&engine, pScene);
#endif

#ifdef TEST_PBR_RENDER
		PBR_MODEL(&engine, pScene);
#endif


		MEntity* pDirLight = pScene->CreateEntity();
		pDirLight->SetName("DirectionalLight");
		if (MSceneComponent* pSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
		{
			//pSceneComponent->SetRotation(Quaternion(Vector3(1.0, 0.0, 0.0), 45.0f));
		}
		pDirLight->RegisterComponent<MDirectionalLightComponent>();



	}

	//start run
	engine.Start();

	while (!bClosed)
	{
		engine.Update();
	}

	editor.Release();

	//stop run
	engine.Stop();

	//release
	engine.Release();






}
