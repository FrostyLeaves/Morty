#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResourceUtil.h"

void ADD_DIRECTIONAL_LIGHT(MEngine* pEngine, MScene* pScene)
{
	MEntity* pDirLight = pScene->CreateEntity();
	pDirLight->SetName("DirectionalLight");
	if (MSceneComponent* pSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetRotation(Quaternion(Vector3(1.0, 0.0, 0.0), 0.0f));
	}
	if (MDirectionalLightComponent* pLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
	{
		pLightComponent->SetLightIntensity(1.0f);
	}
}

void SHADOW_MAP_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->Load(MMeshResourceUtil::CreateSphere());

	std::shared_ptr<MMaterialResource> pForwardMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	{
		pForwardMaterial->LoadVertexShader("Shader/model.mvs");
		pForwardMaterial->LoadPixelShader("Shader/model.mps");
		pForwardMaterial->SetMaterialType(MEMaterialType::EDefault);

		pForwardMaterial->GetMaterialParamSet()->SetValue("f3Ambient", Vector3(1.0f, 1.0f, 1.0f));
		pForwardMaterial->GetMaterialParamSet()->SetValue("f3Diffuse", Vector3(1.0f, 1.0f, 1.0f));
		pForwardMaterial->GetMaterialParamSet()->SetValue("f3Specular", Vector3(1.0f, 1.0f, 1.0f));
		pForwardMaterial->GetMaterialParamSet()->SetValue("fAlphaFactor", 1.0f);
		pForwardMaterial->GetMaterialParamSet()->SetValue("fShininess", 32.0f);

		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		pForwardMaterial->SetTexture("u_texDiffuse", diffuse);
		pForwardMaterial->SetTexture("u_texNormal", normal);
	}


	std::shared_ptr<MMaterialResource> pDeferredMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	{
		pDeferredMaterial->LoadVertexShader("Shader/model_gbuffer.mvs");
		pDeferredMaterial->LoadPixelShader("Shader/model_gbuffer.mps");
		pDeferredMaterial->SetMaterialType(MEMaterialType::EDeferred);

		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

		pDeferredMaterial->SetTexture(MaterialKey::Albedo, albedo);
		pDeferredMaterial->SetTexture(MaterialKey::Normal, normal);
		pDeferredMaterial->SetTexture(MaterialKey::Metallic, metal);
		pDeferredMaterial->SetTexture(MaterialKey::Roughness, roughness);
		pDeferredMaterial->SetTexture(MaterialKey::AmbientOcc, ao);
		pDeferredMaterial->SetTexture(MaterialKey::Height, height);

		pDeferredMaterial->GetMaterialParamSet()->SetValue("fMetallic", 1.0f);
		pDeferredMaterial->GetMaterialParamSet()->SetValue("fRoughness", 1.0f);
		pDeferredMaterial->GetMaterialParamSet()->SetValue("f4Albedo", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	MEntity* pFloorEntity = pScene->CreateEntity();
	pFloorEntity->SetName("Floor");
	if (MSceneComponent* pSceneComponent = pFloorEntity->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(0, 0, 0));
		pSceneComponent->SetScale(Vector3(100.0f, 1.0f, 100.0f));
	}
	if (MRenderableMeshComponent* pMeshComponent = pFloorEntity->RegisterComponent<MRenderableMeshComponent>())
	{
		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

		pMaterial->LoadVertexShader("Shader/model.mvs");
		pMaterial->LoadPixelShader("Shader/model.mps");
		pMaterial->SetMaterialType(MEMaterialType::EDefault);

		pMaterial->GetMaterialParamSet()->SetValue("f3Ambient", Vector3(0.5f, 0.5f, 0.5f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Diffuse", Vector3(0.5f, 0.5f, 0.5f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Specular", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("fAlphaFactor", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fShininess", 32.0f);

		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		pMaterial->SetTexture("u_texDiffuse", diffuse);
		pMaterial->SetTexture("u_texNormal", normal);

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
			if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
			{
				pMeshComponent->SetGenerateDirLightShadow(true);
				pMeshComponent->SetShadowType(MRenderableMeshComponent::MEShadowType::EOnlyDirectional);
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
			pSceneComponent->SetScale(Vector3(20.0f, 200.0f, 2.0f));
			pSceneComponent->SetParentComponent(pCubeFolderComponent->GetComponentID());
		}
		if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
		{
			pMeshComponent->SetGenerateDirLightShadow(true);
			pMeshComponent->SetShadowType(MRenderableMeshComponent::MEShadowType::EOnlyDirectional);

			std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
			pCubeResource->Load(MMeshResourceUtil::CreateCube(MEMeshVertexType::Normal));
			pMeshComponent->Load(pCubeResource);
			
			pMeshComponent->SetMaterial(pForwardMaterial);
		}
	}


}
