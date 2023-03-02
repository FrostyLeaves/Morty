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


void LOAD_MODEL_SPONZA_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();

	std::shared_ptr<MResource> pModelResource = pResourceSystem->LoadResource("./Sponza/Sponza/Sponza.entity");
	if (!pModelResource)
	{
		MModelConverter convert(pEngine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_PBR_Deferred;
		info.strOutputDir = "./Sponza";
		info.strOutputName = "Sponza";
		info.strResourcePath = "./Model/Sponza/NewSponza_Main_glTF_002.gltf";
		info.bImportCamera = false;
		info.bImportLights = false;

		convert.Convert(info);

		pModelResource = pResourceSystem->LoadResource("./Sponza/Sponza/Sponza.entity");
	}

	std::vector<MComponentID> vMeshComponents;
	for (size_t i = 0; i < 1; ++i)
	{
		auto&& vEntity = pEntitySystem->LoadEntity(pScene, pModelResource);

		for (MEntity* pEntity : vEntity)
		{
			pEntitySystem->FindAllComponentRecursively(pEntity, MRenderableMeshComponent::GetClassType(), vMeshComponents);
		}
	}

	std::set<std::shared_ptr<MMaterial>> tMaterials;

	for (MComponentID& componentID : vMeshComponents)
	{
		if (MRenderableMeshComponent* pMeshComponent = pScene->GetComponent(componentID)->DynamicCast<MRenderableMeshComponent>())
		{
			if (auto pMaterial = pMeshComponent->GetMaterial())
			{
				tMaterials.insert(pMaterial);
			}

			pMeshComponent->SetGenerateDirLightShadow(true);
		//	pMeshComponent->SetBatchInstanceEnable(true);
		}

	}

	/*
	for (auto pMaterial : tMaterials)
	{
		pMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::DRAW_MESH_MERGE_INSTANCING);
		pMaterial->LoadVertexShader(pMaterial->GetVertexShaderResource());
		pMaterial->LoadPixelShader(pMaterial->GetPixelShaderResource());
	}
	*/
}
