#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"


void GPU_DRIVEN_CULLING_TEST(MEngine* pEngine, MScene* pScene)
{

	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
	pMeshResource->LoadAsSphere();

	std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

	pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_MERGE_INSTANCING);
	pMaterial->LoadVertexShader("Shader/model_gbuffer.mvs");
	pMaterial->LoadPixelShader("Shader/model_gbuffer.mps");
	pMaterial->SetMaterialType(MEMaterialType::EDeferred);

	std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
	std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
	std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_roughness.png");
	std::shared_ptr<MResource> ao = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_ao.png");
	std::shared_ptr<MResource> height = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_height.png");
	std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);

	pMaterial->SetTexture(MaterialKey::Albedo, albedo);
	pMaterial->SetTexture(MaterialKey::Normal, normal);
	pMaterial->SetTexture(MaterialKey::Metallic, metal);
	pMaterial->SetTexture(MaterialKey::Roughness, roughness);
	pMaterial->SetTexture(MaterialKey::AmbientOcc, ao);
	pMaterial->SetTexture(MaterialKey::Height, height);

	pMaterial->GetMaterialParamSet()->SetValue("fMetallic", 1.0f);
	pMaterial->GetMaterialParamSet()->SetValue("fRoughness", 1.0f);


	MEntity* pFolderEntity = pScene->CreateEntity();
	MSceneComponent* pFolderSceneComponent = pFolderEntity->RegisterComponent<MSceneComponent>();

	for (int x = 0; x < 5; ++x)
	{
		for (int y = 0; y < 5; ++y)
		{
			for (int z = 0; z < 40; ++z)
			{
				MEntity* pSphereEntity = pScene->CreateEntity();
				pSphereEntity->SetName("Sphere_Instancing_" + MStringHelper::ToString(x * 1000 + y * 1000 + z));
				if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
				{
					pSceneComponent->SetPosition(Vector3(x * 10, y * 10, z * 10));
					pSceneComponent->SetScale(Vector3(4.0f, 4.0f, 4.0f));
					pSceneComponent->SetParent(pFolderSceneComponent);
				}
				if (MRenderableMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderableMeshComponent>())
				{
					pMeshComponent->Load(pMeshResource);
					pMeshComponent->SetMaterial(pMaterial);
					pMeshComponent->SetBatchInstanceEnable(true);
				}
			}
		}
	}
}

