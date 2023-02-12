#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"

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


		pMaterial->LoadVertexShader("Shader/model.mvs");
		pMaterial->LoadPixelShader("Shader/model.mps");

		pMaterial->GetMaterialParamSet()->SetValue("f3Ambient", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Diffuse", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("f3Specular", Vector3(1.0f, 1.0f, 1.0f));
		pMaterial->GetMaterialParamSet()->SetValue("fAlphaFactor", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fShininess", 32.0f);


		std::shared_ptr<MResource> diffuse = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
		pMaterial->SetTexture("u_texDiffuse", diffuse);
		pMaterial->SetTexture("u_texNormal", normal);

		if (normal)
		{
			pMaterial->GetMaterialParamSet()->SetValue("bUseNormalTex", 1);
		}

		pMeshComponent->Load(pCubeResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}
