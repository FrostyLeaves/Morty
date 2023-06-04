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
#include "Model/MTextureConverter.h"
#include "Utility/MTimer.h"


class MSponzaTextureDelegate : public MITextureDelegate
{
public:

	MSponzaTextureDelegate(MEngine* pEngine) : m_pEngine(pEngine) {}

	std::shared_ptr<MTextureResource> GetTexture(const MString& strFullPath, MEModelTextureUsage eUsage) override
	{
		std::pair<MString, MEModelTextureUsage> key(strFullPath, eUsage);

		if (m_tTextures.find(key) != m_tTextures.end())
		{
			return m_tTextures[key];
		}

		MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

		auto pResource = pResourceSystem->LoadResource(strFullPath);
		if (!pResource)
		{
			MORTY_ASSERT(pResource);
			return nullptr;
		}

		std::shared_ptr<MTextureResource> pTexture = MTypeClass::DynamicCast<MTextureResource>(pResource);

		if (eUsage == MEModelTextureUsage::Metallic)
		{
			pTexture = MTextureConverter::ConvertSingleChannel(pTexture, 2);
		}
		else if (eUsage == MEModelTextureUsage::Roughness)
		{	
			pTexture = MTextureConverter::ConvertSingleChannel(pTexture, 1);
		}

		m_tTextures[key] = pTexture;

		return pTexture;
	}
	

private:

	std::map<std::pair<MString, MEModelTextureUsage>, std::shared_ptr<MTextureResource> > m_tTextures;

	MEngine* m_pEngine = nullptr;
};

void LOAD_MODEL_SPONZA_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();


	auto time = MTimer::GetCurTime();

	std::shared_ptr<MResource> pModelResource = pResourceSystem->LoadResource("./Sponza/Sponza/Sponza.entity");

	time = MTimer::GetCurTime() - time;

	pEngine->GetLogger()->Log("Load sponza scene time: %lld", time);

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
		info.pTextureDelegate = std::make_shared<MSponzaTextureDelegate>(pEngine);

		convert.Convert(info);

		pModelResource = pResourceSystem->LoadResource("./Sponza/Sponza/Sponza.entity");
	}

	std::vector<MComponentID> vMeshComponents;
	for (size_t i = 0; i < 1; ++i)
	{
		auto vEntity = pEntitySystem->LoadEntity(pScene, pModelResource);

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
		}

	}
	
}
