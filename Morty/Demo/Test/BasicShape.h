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
	if (MRenderMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderMeshComponent>())
	{
		std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();


		pMaterial->LoadShader("Shader/Model/universal_model.mvs");
		pMaterial->LoadShader("Shader/Forward/basic_lighting.mps");

		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_AMBIENT, Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_DIFFUSE, Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SPECULAR, Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALPHA_FACTOR, 1.0f);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SHININESS, 32.0f);


		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_DIFFUSE, diffuse);
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);

		if (normal)
		{
			pMaterial->GetMaterialPropertyBlock()->SetValue("bUseNormalTex", 1);
		}

		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}
