#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "Utility/MGlobal.h"

void ADD_DIRECTIONAL_LIGHT(MEngine* pEngine, MScene* pScene)
{
	MORTY_UNUSED(pEngine);
	
	MEntity* pDirLight = pScene->CreateEntity();
	pDirLight->SetName("DirectionalLight");
	if (MSceneComponent* pSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetRotation(Quaternion(Vector3(1.0, 0.0, 0.0), 45.0f));
	}
	if (MDirectionalLightComponent* pLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
	{
		pLightComponent->SetLightIntensity(1.0f);
	}
}

void SHADOW_MAP_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->Load(MMeshResourceUtil::CreateSphere());

	std::shared_ptr<MMaterialResource> pForwardMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	{
		pForwardMaterial->LoadShader("Shader/Model/universal_model.mvs");
		pForwardMaterial->LoadShader("Shader/Forward/basic_lighting.mps");
		pForwardMaterial->SetMaterialType(MEMaterialType::EDefault);

		pForwardMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_AMBIENT, Vector3(1.0f, 1.0f, 1.0f));
		pForwardMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_DIFFUSE, Vector3(1.0f, 1.0f, 1.0f));
		pForwardMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SPECULAR, Vector3(1.0f, 1.0f, 1.0f));
		pForwardMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALPHA_FACTOR, 1.0f);
		pForwardMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SHININESS, 32.0f);

		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		pForwardMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_DIFFUSE, diffuse);
		pForwardMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);
	}


	std::shared_ptr<MMaterialResource> pDeferredMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	{
		pDeferredMaterial->LoadShader("Shader/Model/universal_model.mvs");
		pDeferredMaterial->LoadShader("Shader/Deferred/deferred_gbuffer.mps");
		pDeferredMaterial->SetMaterialType(MEMaterialType::EDeferred);

		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

		pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO, albedo);
		pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);
		pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_METALLIC, metal);
		pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS, roughness);
		pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC, ao);
		pDeferredMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT, height);

		pDeferredMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_METALLIC, 1.0f);
		pDeferredMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ROUGHNESS, 1.0f);
		pDeferredMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALBEDO, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	MEntity* pFloorEntity = pScene->CreateEntity();
	pFloorEntity->SetName("Floor");
	if (MSceneComponent* pSceneComponent = pFloorEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(0, 0, 0));
		pSceneComponent->SetScale(Vector3(100.0f, 1.0f, 100.0f));
	}
	if (MRenderMeshComponent* pMeshComponent = pFloorEntity->RegisterComponent<MRenderMeshComponent>())
	{
		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

		pMaterial->LoadShader("Shader/Model/universal_model.mvs");
		pMaterial->LoadShader("Shader/Forward/basic_lighting.mps");
		pMaterial->SetMaterialType(MEMaterialType::EDefault);

		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_AMBIENT, Vector3(0.5f, 0.5f, 0.5f));
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_DIFFUSE, Vector3(0.5f, 0.5f, 0.5f));
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SPECULAR, Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALPHA_FACTOR, 1.0f);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SHININESS, 32.0f);

		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_DIFFUSE, diffuse);
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);

		std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
		pCubeResource->Load(MMeshResourceUtil::CreateCube(MEMeshVertexType::Normal));
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
				pSceneComponent->SetPosition(Vector3(x * distance + 20, 10, y * distance));
				pSceneComponent->SetScale(Vector3(1.0f, 1.0f, 1.0f) * (y + 1)); 
				pSceneComponent->SetParentComponent(pCubeFolderComponent->GetComponentID());
			}
			if (MRenderMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderMeshComponent>())
			{
				pMeshComponent->SetGenerateDirLightShadow(true);
				pMeshComponent->SetShadowType(MRenderMeshComponent::MEShadowType::EOnlyDirectional);
				pMeshComponent->Load(pCubeResource);

				if (x % 2 == 0)
				{
					pMeshComponent->SetMaterial(pForwardMaterial);
				}
				else
				{
					pMeshComponent->SetMaterial(pDeferredMaterial);
				}
			}
		}
	}


	{
		MEntity* pSphereEntity = pScene->CreateEntity();
		pSphereEntity->SetName("Cube");
		if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
		{
			pSceneComponent->SetPosition(Vector3(-5 + 20, 5, 40));
			pSceneComponent->SetScale(Vector3(20.0f, 20.0f, 2.0f));
			pSceneComponent->SetParentComponent(pCubeFolderComponent->GetComponentID());
		}
		if (MRenderMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderMeshComponent>())
		{
			pMeshComponent->SetGenerateDirLightShadow(true);
			pMeshComponent->SetShadowType(MRenderMeshComponent::MEShadowType::EOnlyDirectional);

			std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
			pCubeResource->Load(MMeshResourceUtil::CreateCube(MEMeshVertexType::Normal));
			pMeshComponent->Load(pCubeResource);
			
			pMeshComponent->SetMaterial(pForwardMaterial);
		}
	}


}
