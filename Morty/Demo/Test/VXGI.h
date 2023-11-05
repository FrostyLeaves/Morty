#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"
#include "Widget/ModelConvertView.h"

void VXGI_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MMeshResource> pCubeMeshResource = pResourceSystem->CreateResource<MMeshResource>();
	pCubeMeshResource->Load(MMeshResourceUtil::CreateSphere());
	
	std::shared_ptr<MResource> albedo = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
	std::shared_ptr<MResource> normal = pResourceSystem->LoadResource(MRenderModule::DefaultNormal);
	std::shared_ptr<MResource> roughness = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
	std::shared_ptr<MResource> ao = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
	std::shared_ptr<MResource> metal = pResourceSystem->LoadResource(MRenderModule::Default_R8_One);
	std::shared_ptr<MResource> height = pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero);

	auto CreateCubeFunction = [&](MString name, Vector3 position, Vector3 color, float fMetallic, float fRoughness)
	{
		MEntity* pSphereEntity = pScene->CreateEntity();
		pSphereEntity->SetName(name);
		if (MSceneComponent* pSceneComponent = pSphereEntity->RegisterComponent<MSceneComponent>())
		{
			pSceneComponent->SetPosition(position);
			pSceneComponent->SetScale(Vector3(3.0f));
		}
		if (MRenderMeshComponent* pMeshComponent = pSphereEntity->RegisterComponent<MRenderMeshComponent>())
		{
			std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

			pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
			pMaterial->LoadShader("Shader/Model/universal_model.mvs");
			pMaterial->LoadShader("Shader/Deferred/deferred_gbuffer.mps");
			pMaterial->SetMaterialType(MEMaterialType::EDeferred);

			pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO, albedo);
			pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, normal);
			pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_METALLIC, metal);
			pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS, roughness);
			pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC, ao);
			pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT, height);

			auto material = pMaterial->GetMaterialPropertyBlock()->FindConstantParam(MShaderPropertyName::MATERIAL_CBUFFER_NAME);

			MStruct materialSut = material->var.GetValue<MStruct>();
			MStruct uxMaterial = materialSut.GetVariant<MStruct>(MShaderPropertyName::MATERIAL_STRUCT_NAME);

			Vector3 xyz;
			spectrum_to_xyz(bb_spectrum, &xyz.x, &xyz.y, &xyz.z);

			uxMaterial.SetVariant(MShaderPropertyName::MATERIAL_METALLIC, fMetallic);
			uxMaterial.SetVariant(MShaderPropertyName::MATERIAL_ROUGHNESS, fRoughness);
			uxMaterial.SetVariant(MShaderPropertyName::MATERIAL_ALBEDO, Vector4(color, 1.0f));

			pMeshComponent->Load(pCubeMeshResource);
			pMeshComponent->SetMaterial(pMaterial);
		}

		return pSphereEntity;
	};



	CreateCubeFunction("left", Vector3(-10, 0, 0), Vector3(1, 0, 0), 1.0f, 1.0f);
	CreateCubeFunction("right", Vector3(+10, 0, 0), Vector3(0, 1, 0), 1.0f, 1.0f);
	CreateCubeFunction("forward", Vector3(0, 0, 10), Vector3(0, 0, 1), 1.0f, 1.0f);


	CreateCubeFunction("center", Vector3(0, 0, 0), Vector3(1, 1, 1), 1.0f, 1.0f);
	
}
