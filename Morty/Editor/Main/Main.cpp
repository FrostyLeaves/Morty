// Demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "SDL.h"
#include <fstream>

#include "Engine/MEngine.h"
#include "MainEditor.h"

#include "MRenderModule.h"
#include "Model/MModelConverter.h"

#include "Module/MEditorModule.h"

#include "System/MEntitySystem.h"
#include "System/MSkyBoxSystem.h"
#include "System/MResourceSystem.h"
#include "System/MRenderSystem.h"

#include "Material/MMaterial.h"

#include "Scene/MScene.h"
#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Component/MSkyBoxComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Component/MCameraComponent.h"
#include "Component/MDebugRenderComponent.h"

#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"

#ifdef MORTY_WIN
#undef main
#endif

//#define TEST_ANIMATION

//#define TEST_PBR_RENDER_0

//#define TEST_PBR_RENDER_1

//#define TEST_POINT_LIGHT

#define TEST_DIR_LIGHT

//#define TEST_BASIC_SHAPE

#define TEST_SKY_BOX

#define TEST_SHADOW_MAP

void SHADOW_MAP_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->LoadAsCube();

	std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

	pMaterial->LoadVertexShader("./Shader/model_gbuffer.mvs");
	pMaterial->LoadPixelShader("./Shader/model_gbuffer.mps");
	pMaterial->SetMaterialType(MEMaterialType::EDeferred);

	std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
	std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
	std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
	std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
	std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
	std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

	pMaterial->SetTexutreParam(MaterialKey::Albedo, albedo);
	pMaterial->SetTexutreParam(MaterialKey::Normal, normal);
	pMaterial->SetTexutreParam(MaterialKey::Metallic, metal);
	pMaterial->SetTexutreParam(MaterialKey::Roughness, roughness);
	pMaterial->SetTexutreParam(MaterialKey::AmbientOcc, ao);
	pMaterial->SetTexutreParam(MaterialKey::Height, height);

	pMaterial->GetMaterialParamSet()->SetValue("fMetallic", 1.0f);
	pMaterial->GetMaterialParamSet()->SetValue("fRoughness", 1.0f);


	MEntity* pFloorEntity = pScene->CreateEntity();
	pFloorEntity->SetName("Floor");
	if (MSceneComponent* pSceneComponent = pFloorEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(0, 0, 0));
		pSceneComponent->SetScale(Vector3(1000.0f, 1.0f, 1000.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pFloorEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

		pMaterial->LoadVertexShader("./Shader/model_gbuffer.mvs");
		pMaterial->LoadPixelShader("./Shader/model_gbuffer.mps");
		pMaterial->SetMaterialType(MEMaterialType::EDeferred);

		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

		pMaterial->SetTexutreParam(MaterialKey::Albedo, albedo);
		pMaterial->SetTexutreParam(MaterialKey::Normal, normal);
		pMaterial->SetTexutreParam(MaterialKey::Metallic, metal);
		pMaterial->SetTexutreParam(MaterialKey::Roughness, roughness);
		pMaterial->SetTexutreParam(MaterialKey::AmbientOcc, ao);
		pMaterial->SetTexutreParam(MaterialKey::Height, height);

		pMaterial->GetMaterialParamSet()->SetValue("fMetallic", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fRoughness", 1.0f);

		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}

	MEntity* pCubeFolder = pScene->CreateEntity();
	pCubeFolder->SetName("Cube Folder");
	MSceneComponent* pCubeFolderComponent = pCubeFolder->RegisterComponent<MSceneComponent>();

	const float distance = 10;
	const size_t nNum = 2;
	for (size_t x = 0; x < nNum; ++x)
	{
		for (size_t y = 0; y < nNum; ++y)
		{
			MEntity* pSphereEntity = pScene->CreateEntity();
			pSphereEntity->SetName("Sphere_" + std::to_string(x) + "_" + std::to_string(y));
			if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
			{
				pSceneComponent->SetPosition(Vector3(x * distance, 10, y * distance));
				pSceneComponent->SetScale(Vector3(1.0f, 1.0f, 1.0f));
				pSceneComponent->SetParentComponent(pCubeFolderComponent->GetComponentID());
			}
			if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
			{
				pMeshComponent->SetGenerateDirLightShadow(true);
				pMeshComponent->SetShadowType(MRenderableMeshComponent::MEShadowType::EOnlyDirectional);
				pMeshComponent->Load(pCubeResource);
				pMeshComponent->SetMaterial(pMaterial);
			}
		}

	}
}

void SKY_BOX(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();
	MSkyBoxSystem* pSkyBoxSystem = pEngine->FindSystem<MSkyBoxSystem>();


	std::shared_ptr<MTextureResource> pCubeTexture = pResourceSystem->CreateResource<MTextureResource>();
	pCubeTexture->ImportCubeMap({
		"Texture/Sky/Circus_Backstage/px.hdr",
		"Texture/Sky/Circus_Backstage/nx.hdr",
		"Texture/Sky/Circus_Backstage/py.hdr",
		"Texture/Sky/Circus_Backstage/ny.hdr",
		"Texture/Sky/Circus_Backstage/pz.hdr",
		"Texture/Sky/Circus_Backstage/nz.hdr"
		},{ MTextureResource::PixelFormat::Float32 });

/*
	std::shared_ptr<MTextureResource> pEnvironment = pResourceSystem->CreateResource<MTextureResource>();
	pEnvironment->ImportCubeMap({
		"Texture/Sky/Circus_Backstage/Env/px.hdr",
		"Texture/Sky/Circus_Backstage/Env/nx.hdr",
		"Texture/Sky/Circus_Backstage/Env/py.hdr",
		"Texture/Sky/Circus_Backstage/Env/ny.hdr",
		"Texture/Sky/Circus_Backstage/Env/pz.hdr",
		"Texture/Sky/Circus_Backstage/Env/nz.hdr"
		}, {MTextureResource::PixelFormat::Float32});
*/
	MEntity* pSkyBoxEntity = pScene->CreateEntity();
	pSkyBoxEntity->SetName("SkyBox");
	
	if (MSkyBoxComponent* pSkyBoxComponent = pSkyBoxEntity->RegisterComponent<MSkyBoxComponent>())
	{
		pSkyBoxComponent->LoadSkyBoxResource(pCubeTexture);
		//pSkyBoxComponent->LoadDiffuseEnvResource(pEnvironment);

		pSkyBoxSystem->GenerateEnvironmentTexture(pSkyBoxComponent);
	}
}

void SPHERE_GENERATE(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->LoadAsCube();

	MEntity* pSphereEntity = pScene->CreateEntity();
	pSphereEntity->SetName("Sphere");
	if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();


		pMaterial->LoadVertexShader("./Shader/model.mvs");
		pMaterial->LoadPixelShader("./Shader/model.mps");

		pMaterial->GetMaterialParamSet()->SetValue("f3Ambient", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Diffuse", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Specular", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("fAlphaFactor", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fShininess", 32.0f);


		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
		pMaterial->SetTexutreParam("U_mat_texDiffuse", diffuse);
		pMaterial->SetTexutreParam("U_mat_texNormal", normal);

		if (normal)
		{
			pMaterial->GetMaterialParamSet()->SetValue("bUseNormalTex", 1);
		}

		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}

void PBR_SHPERE(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MResource> pCubeMeshResource = nullptr;
	
	if (true)
	{
		std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
		pMeshResource->LoadAsSphere();
		pCubeMeshResource = pMeshResource;
	}
	else
	{
		pCubeMeshResource = pResourceSystem->LoadResource("D:/test_sphere/sphere/GeoSphere001_0.mesh");
		if (!pCubeMeshResource)
		{
			MModelConverter convert(pEngine);

			MModelConvertInfo info;
			info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;
			info.strOutputDir = "D:/test_sphere";
			info.strOutputName = "sphere";
			info.strResourcePath = "./Model/sphere.fbx";

			convert.Convert(info);

			pCubeMeshResource = pResourceSystem->LoadResource("D:/test_sphere/sphere/GeoSphere001_0.mesh");
		}
	}

	MEntity* pSphereEntity = pScene->CreateEntity();
	pSphereEntity->SetName("Sphere_PBR");
	if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

		pMaterial->LoadVertexShader("./Shader/model_gbuffer.mvs");
		pMaterial->LoadPixelShader("./Shader/model_gbuffer.mps");
		pMaterial->SetMaterialType(MEMaterialType::EDeferred);

#if true
		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_roughness.png");
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_ao.png");
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource("./Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_height.png");
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
#else
		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);
#endif

		pMaterial->SetTexutreParam(MaterialKey::Albedo, albedo);
		pMaterial->SetTexutreParam(MaterialKey::Normal, normal);
		pMaterial->SetTexutreParam(MaterialKey::Metallic, metal);
		pMaterial->SetTexutreParam(MaterialKey::Roughness, roughness);
		pMaterial->SetTexutreParam(MaterialKey::AmbientOcc, ao);
		pMaterial->SetTexutreParam(MaterialKey::Height, height);

		pMaterial->GetMaterialParamSet()->SetValue("fMetallic", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fRoughness", 1.0f);

		pMeshComponent->Load(pCubeMeshResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}

void PBR_MODEL(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();


	std::shared_ptr<MResource> pDroneResource = pResourceSystem->LoadResource("D:/test_drone/drone/drone.entity");
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
	std::shared_ptr<MResource> pIconTexture = pResourceSystem->LoadResource("./Texture/Icon/point_light.png");

	std::shared_ptr<MMeshResource> pPanelMesh = pResourceSystem->CreateResource<MMeshResource>();
	pPanelMesh->LoadAsPlane(MMeshResource::MEMeshVertexType::Normal, Vector3(10.0f, 10.0f, 1.0f));

	std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

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
		MRenderSystem* pRenderSystem = engine.FindSystem<MRenderSystem>();


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

#ifdef TEST_DIR_LIGHT
		MEntity* pDirLight = pScene->CreateEntity();
		pDirLight->SetName("DirectionalLight");
		if (MSceneComponent* pSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
		{
			//pSceneComponent->SetRotation(Quaternion(Vector3(1.0, 0.0, 0.0), 45.0f));
		}
		if (MDirectionalLightComponent* pLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
		{
			pLightComponent->SetLightIntensity(1.0f);
		}
#endif


#ifdef TEST_SHADOW_MAP

		SHADOW_MAP_TEST(&engine, pScene);

#endif


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
