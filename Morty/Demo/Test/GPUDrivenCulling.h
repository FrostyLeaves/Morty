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

void GPU_DRIVEN_CULLING_TEST(MEngine* pEngine, MScene* pScene)
{

	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
	pMeshResource->Load(MMeshResourceUtil::CreateSphere());

	const auto pTemplate = pResourceSystem->CreateResource<MMaterialTemplate>();

	pTemplate->AddDefine(MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	pTemplate->LoadShader("Shader/Model/universal_model.mvs");
	pTemplate->LoadShader("Shader/Deferred/deferred_gbuffer.mps");
	pTemplate->SetMaterialType(MEMaterialType::EDeferred);

	const auto pMaterial = MMaterialResource::CreateMaterial(pTemplate);

	std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
	std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
	std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_roughness.png");
	std::shared_ptr<MResource> ao = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_ao.png");
	std::shared_ptr<MResource> height = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_height.png");
	std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);

	pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO, albedo);
	pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);
	pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_METALLIC, metal);
	pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS, roughness);
	pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC, ao);
	pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT, height);

	pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_METALLIC, 1.0f);
	pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ROUGHNESS, 1.0f);
	pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALBEDO, Vector4(1.0f, 1.0f, 1.0f, 1.0f));



	MEntity* pFolderEntity = pScene->CreateEntity();
	MSceneComponent* pFolderSceneComponent = pFolderEntity->RegisterComponent<MSceneComponent>();

	for (int x = 0; x < 5; ++x)
	{
		for (int y = 0; y < 5; ++y)
		{
			for (int z = 0; z < 40; ++z)
			{
				MEntity* pSphereEntity = pScene->CreateEntity();
				pSphereEntity->SetName("Sphere_Instancing_" + MStringUtil::ToString(x * 1000 + y * 1000 + z));
				if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
				{
					pSceneComponent->SetPosition(Vector3(x * 10, y * 10, z * 10));
					pSceneComponent->SetScale(Vector3(4.0f, 4.0f, 4.0f));
					pSceneComponent->SetParent(pFolderSceneComponent);
				}
				if (MRenderMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderMeshComponent>())
				{
					pMeshComponent->Load(pMeshResource);
					pMeshComponent->SetMaterial(pMaterial);
				}
			}
		}
	}
}

