#define DOCTEST_CONFIG_IMPLEMENT

#include "SDL.h"
#include <fstream>

#include "Engine/MEngine.h"
#include "Main/MainEditor.h"

#include "Module/MCoreModule.h"
#include "MRenderModule.h"
#include "Module/MEditorModule.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "System/MRenderSystem.h"

#include "Scene/MScene.h"

#ifdef MORTY_WIN
#undef main
#endif


#include "System/MObjectSystem.h"
#include "Test/ShadowMap.h"
#include "Test/EnvironmentCubemap.h"
#include "Test/GPUDrivenCulling.h"
#include "Test/LoadModel.h"
#include "Test/LoadModel_Sponza.h"
#include "Test/Pbr.h"
#include "Test/Floor.h"
#include "Test/BasicTransform.h"

int main()
{
	//initialize
	MEngine engine;
	engine.Initialize();

	//register module
	MCoreModule::Register(&engine);
	MRenderModule::Register(&engine);
	MEditorModule::Register(&engine);

	//create window.
	SDLRenderView renderView;
	renderView.Initialize(&engine);

	//attach to window surface.
	renderView.BindSDLWindow();

	//create editor
	MainEditor* editor = new MainEditor();
	editor->Initialize(&engine);
	renderView.AppendContent(editor);

	//create a scene.
	MScene* pScene = engine.FindSystem<MObjectSystem>()->CreateObject<MScene>();
	editor->SetScene(pScene);

	ADD_DIRECTIONAL_LIGHT(&engine, pScene);
	CREATE_FLOOR_GRID(&engine, pScene);
	// 	ENVIRONMENT_CUBEMAP_TEST(&engine, pScene);
	//	SHADOW_MAP_TEST(&engine, pScene);
	//	PBR_SHPERE(&engine, pScene);
	//	LOAD_MODEL_ANIMATION_TEST(&engine, pScene);
	//	LOAD_MODEL_TRANSLATION_TEST(&engine, pScene);
	//	LOAD_MODEL_SPONZA_TEST(&engine, pScene);
	//	GPU_DRIVEN_CULLING_TEST(&engine, pScene);
	//	TRANSFORM_SPHERE_GENERATE(&engine, pScene);

	{
		MResourceSystem* pResourceSystem = engine.FindSystem<MResourceSystem>();

		MEntity* pSphereEntity = pScene->CreateEntity();
		pSphereEntity->SetName("Sphere");
		if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
		{
			pSceneComponent->SetPosition(Vector3(1.0f, 1.0f, 1.0f));
		}
		if (MRenderMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderMeshComponent>())
		{
			std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

			pMaterial->LoadVertexShader("Shader/Deferred/model_gbuffer.mvs");
			pMaterial->LoadPixelShader("Shader/Deferred/model_gbuffer.mps");
			pMaterial->SetMaterialType(MEMaterialType::EDeferred);

			std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
			std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
			std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
			std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
			std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
			std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

			pMaterial->SetTexture(MaterialKey::Albedo, albedo);
			pMaterial->SetTexture(MaterialKey::Normal, normal);
			pMaterial->SetTexture(MaterialKey::Metallic, metal);
			pMaterial->SetTexture(MaterialKey::Roughness, roughness);
			pMaterial->SetTexture(MaterialKey::AmbientOcc, ao);
			pMaterial->SetTexture(MaterialKey::Height, height);

			auto material = pMaterial->GetMaterialPropertyBlock()->FindConstantParam("cbMaterial");

			MStruct materialSut = material->var.GetValue<MStruct>();
			MStruct uxMaterial = materialSut.GetVariant<MStruct>("u_xMaterial");

			uxMaterial.SetVariant("fMetallic", 1.0f);
			uxMaterial.SetVariant("fRoughness", 1.0f);
			uxMaterial.SetVariant("f4Albedo", Vector4(1.0f, 1.0f, 1.0f, 1.0f));

			std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
			pMeshResource->Load(MMeshResourceUtil::CreateSphere());

			pMeshComponent->Load(pMeshResource);
			pMeshComponent->SetMaterial(pMaterial);
		}
	}

	//start run
	engine.Start();

	while (!renderView.GetClosed())
	{
		engine.Update();
	}

	//stop run
	engine.Stop();

	//destroy editor
	editor->Release();
	delete editor;
	editor = nullptr;

	//destroy window
	renderView.UnbindSDLWindow();
	renderView.Release();

	//release engine
	engine.Release();

}
