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

#include "MSkyBoxComponent.h"
#include "MPointLightComponent.h"
#include "MDirectionalLightComponent.h"

#include "MTextureResource.h"
#include "MMaterialResource.h"

#ifdef MORTY_WIN
#undef main
#endif

//#define TEST_ANIMATION

//#define TEST_PBR_RENDER_0

//#define TEST_PBR_RENDER_1

//#define TEST_POINT_LIGHT

//#define TEST_BASIC_SHAPE

#define TEST_SKY_BOX

void SKY_BOX(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();


	MTextureResource* pCubeTexture = pResourceSystem->CreateResource<MTextureResource>();
	pCubeTexture->ImportCubeMap({
		"Texture/Sky/Circus_Backstage/px.hdr",
		"Texture/Sky/Circus_Backstage/nx.hdr",
		"Texture/Sky/Circus_Backstage/py.hdr",
		"Texture/Sky/Circus_Backstage/ny.hdr",
		"Texture/Sky/Circus_Backstage/pz.hdr",
		"Texture/Sky/Circus_Backstage/nz.hdr"
		},{ MTextureResource::PixelFormat::Float32 });


	MEntity* pSkyBoxEntity = pScene->CreateEntity();
	pSkyBoxEntity->SetName("SkyBox");
	
	if (MSkyBoxComponent* pSkyBoxComponent = pSkyBoxEntity->RegisterComponent<MSkyBoxComponent>())
	{
		pSkyBoxComponent->LoadTexture(pCubeTexture);
	}
}

void SPHERE_GENERATE(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	MMeshResource* pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->LoadAsPlane();

	MEntity* pSphereEntity = pScene->CreateEntity();
	pSphereEntity->SetName("Sphere");
	if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		MMaterialResource* pMaterial = pResourceSystem->CreateResource<MMaterialResource>();


		pMaterial->LoadVertexShader("./Shader/model.mvs");
		pMaterial->LoadPixelShader("./Shader/model.mps");

		pMaterial->GetMaterialParamSet()->SetValue("f3Ambient", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Diffuse", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Specular", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("fAlphaFactor", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fShininess", 32.0f);


		MResource* pTextureResource = pResourceSystem->LoadResource("./Texture/test.png");
		pMaterial->SetTexutreParam("U_mat_texDiffuse", pTextureResource);

		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}

void PBR_SHPERE(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	MMeshResource* pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->LoadAsCube();

	MEntity* pSphereEntity = pScene->CreateEntity();
	pSphereEntity->SetName("Sphere_PBR");
	if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		MMaterialResource* pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

		pMaterial->LoadVertexShader("./Shader/model_gbuffer.mvs");
		pMaterial->LoadPixelShader("./Shader/model_gbuffer.mps");
		pMaterial->SetMaterialType(MEMaterialType::EDeferred);

		MResource* albedo = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
		MResource* normal = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
		MResource* roughness = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_roughness.png");
		MResource* ao = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_ao.png");
		MResource* height = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_height.png");
		MResource* metal = pResourceSystem->LoadResource(MRenderModule::DefaultMetal);
		pMaterial->SetTexutreParam("U_mat_texAlbedo", albedo);
		pMaterial->SetTexutreParam("U_mat_texNormal", normal);
		pMaterial->SetTexutreParam("U_mat_texMetallic", metal);
		pMaterial->SetTexutreParam("U_mat_texRoughness", roughness);
		pMaterial->SetTexutreParam("U_mat_texAmbientOcc", ao);
		pMaterial->SetTexutreParam("U_mat_texHeight", height);

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

void ADD_POINT_LIGHT(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MResource* pIconTexture = pResourceSystem->LoadResource("./Texture/Icon/point_light.png");

	MMeshResource* pPanelMesh = pResourceSystem->CreateResource<MMeshResource>();
	pPanelMesh->LoadAsPlane(MMeshResource::MEMeshVertexType::Normal, Vector3(10.0f, 10.0f, 1.0f));

	MMaterialResource* pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

	pMaterial->LoadVertexShader("./Shader/debug_model.mvs");
	pMaterial->LoadPixelShader("./Shader/debug_model.mps");

	pMaterial->SetTexutreParam("U_mat_texDiffuse", pIconTexture);

	for (int i = 0; i < 9; ++i)
	{
		MString strEntityName = "PointLight_" + MStringHelper::ToString(i);

		MEntity* pPointLight = pScene->CreateEntity();
		pPointLight->SetName(strEntityName);

		if (MSceneComponent* pSceneComponent = pPointLight->RegisterComponent<MSceneComponent>())
		{
			pSceneComponent->SetPosition(Vector3(i / 3 * 10.0f, 10.0f, i % 3 * 10.0f));
		}

		if (MPointLightComponent* pPointLightComponent = pPointLight->RegisterComponent<MPointLightComponent>())
		{
			pPointLightComponent->SetLightIntensity(100.0f);
		}

		if (MRenderableMeshComponent* pMeshComponent = pPointLight->RegisterComponent<MRenderableMeshComponent>())
		{
			pMeshComponent->Load(pPanelMesh);
			pMeshComponent->SetMaterial(pMaterial);
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


#ifdef	TEST_BASIC_SHAPE
		SPHERE_GENERATE(&engine, pScene);
#endif

#ifdef	TEST_ANIMATION
		ANIMATION_MODEL(&engine, pScene);
#endif


#ifdef TEST_PBR_RENDER_0
		PBR_SHPERE(&engine, pScene);
#endif

#ifdef TEST_PBR_RENDER_1
		PBR_MODEL(&engine, pScene);
#endif

#ifdef TEST_POINT_LIGHT
		ADD_POINT_LIGHT(&engine, pScene);
#endif

#ifdef TEST_SKY_BOX
		SKY_BOX(&engine, pScene);
#endif

		MEntity* pDirLight = pScene->CreateEntity();
		pDirLight->SetName("DirectionalLight");
		if (MSceneComponent* pSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
		{
			//pSceneComponent->SetRotation(Quaternion(Vector3(1.0, 0.0, 0.0), 45.0f));
		}
		if (MDirectionalLightComponent* pLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
		{
			pLightComponent->SetLightIntensity(250.0f);
		}



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
