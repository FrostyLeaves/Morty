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

int main()
{
	//initialize
	MEngine engine;
	engine.Initialize();

	//register module
	MRenderModule::Register(&engine);
	MEditorModule::Register(&engine);


// 	{
// 		MModelConverter convert(&engine);
// 
// 		MModelConvertInfo info;
// 		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
// 		info.strOutputDir = "D:/test";
// 		info.strOutputName = "banana";
// 		info.strResourcePath = "D:/project/Morty_Restructure/Resource/Model/banana/ripe-banana.obj";
// 
// 		convert.Convert(info);
// 	}

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


		MMeshResource* pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
		pCubeResource->LoadAsCube();

		MEntity* pCubeEntity = pScene->CreateEntity();
		pCubeEntity->SetName("Cube");
		if (MSceneComponent* pSceneComponent = pCubeEntity->RegisterComponent<MSceneComponent>())
		{
			pSceneComponent->SetScale(Vector3(10.0f, 10.0f, 1.0f));
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

			pMeshComponent->Load(pCubeResource);
			pMeshComponent->SetMaterial(pMaterial);
		}

		
		MResource* pResource = pResourceSystem->LoadResource("D:/test/banana/banana.entity");

		std::vector<MComponentID> vMeshComponents;
		for (size_t i = 0; i < 1; ++i)
		{
			auto&& vEntity = pEntitySystem->LoadEntity(pScene, pResource);

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

		if (MRenderableMeshComponent* pMeshComponent = pScene->GetComponent(vMeshComponents[0])->DynamicCast<MRenderableMeshComponent>())
		{
	 		MMaterial* pMaterial = pMeshComponent->GetMaterial();
			pMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
		}

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
