#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"
#include "Widget/ModelConvertView.h"


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

		pMaterial->LoadVertexShader("Shader/model_gbuffer.mvs");
		pMaterial->LoadPixelShader("Shader/model_gbuffer.mps");
		pMaterial->SetMaterialType(MEMaterialType::EDeferred);

#if true
		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_albedo.png");
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_normal.png");
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_roughness.png");
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_ao.png");
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource("Texture/Pbr/Brick/TexturesCom_Brick_Rustic2_1K_height.png");
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
#else
		std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
		std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
		std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);
#endif

		pMaterial->SetTexutre(MaterialKey::Albedo, albedo);
		pMaterial->SetTexutre(MaterialKey::Normal, normal);
		pMaterial->SetTexutre(MaterialKey::Metallic, metal);
		pMaterial->SetTexutre(MaterialKey::Roughness, roughness);
		pMaterial->SetTexutre(MaterialKey::AmbientOcc, ao);
		pMaterial->SetTexutre(MaterialKey::Height, height);

		pMaterial->GetMaterialParamSet()->SetValue("fMetallic", 1.0f);
		pMaterial->GetMaterialParamSet()->SetValue("fRoughness", 1.0f);

		pMeshComponent->Load(pCubeMeshResource);
		pMeshComponent->SetMaterial(pMaterial);
	}
}
