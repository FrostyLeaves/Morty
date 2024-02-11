#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"

using namespace morty;

void TRANSFORM_SPHERE_GENERATE(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();


	const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::BASIC_LIGHTING);
	const auto pForwardMaterial = MMaterialResource::CreateMaterial(pTemplate);
	{
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

	std::shared_ptr<MMeshResource> pCubeResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeResource->Load(MMeshResourceUtil::CreateSphere());

	MEntity* pParent = pScene->CreateEntity();
	pParent->SetName("Parent");
	if (MSceneComponent* pSceneComponent = pParent->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetScale(Vector3(1.0f, 1.0f, 1.0f));
		pSceneComponent->SetRotation(Quaternion::FromEuler(Vector3(0.0f, 0.0f, 0.0f)));
	}

	if (MRenderMeshComponent* pMeshComponent = pParent->RegisterComponent<MRenderMeshComponent>())
	{
		pMeshComponent->SetMaterial(pForwardMaterial);
		pMeshComponent->Load(pCubeResource);
	}


	MEntity* pChild = pScene->CreateEntity();
	pChild->SetName("Child");
	if (MSceneComponent* pSceneComponent = pChild->RegisterComponent<MSceneComponent>())
	{
		pSceneComponent->SetPosition(Vector3(5.0f, 0.0f, 0.0f));
		pSceneComponent->SetScale(Vector3(1.0f, 10.0f, 1.0f));
		pSceneComponent->SetRotation(Quaternion(Vector3(0, 0, 1), 45));
		pSceneComponent->SetParent(pParent->GetComponent<MSceneComponent>());
	}

	if (MRenderMeshComponent* pMeshComponent = pChild->RegisterComponent<MRenderMeshComponent>())
	{
		pMeshComponent->SetMaterial(pForwardMaterial);
		pMeshComponent->Load(pCubeResource);
	}
}
