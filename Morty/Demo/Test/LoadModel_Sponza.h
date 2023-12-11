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
#include "Model/MTextureConverter.h"
#include "Resource/MReadableTextureResource.h"
#include "Utility/MTimer.h"


class MSponzaTextureDelegate : public MITextureDelegate
{
public:

	MSponzaTextureDelegate(MEngine* pEngine) : m_pEngine(pEngine) {}

	std::shared_ptr<MTextureResource> GetTexture(const MString& strFullPath, MEModelTextureUsage eUsage) override
	{
		MString strResourcePath = MFileHelper::ReplaceFileName(strFullPath, MFileHelper::GetFileName(strFullPath) + "_usage_" + MStringUtil::ToString(static_cast<int>(eUsage)));

		if (m_tTextures.find(strResourcePath) != m_tTextures.end())
		{
			return m_tTextures[strResourcePath];
		}

		MResourceSystem* pResourceSystem = m_pEngine->FindSystem<MResourceSystem>();

		auto pResource = pResourceSystem->CreateResource<MReadableTextureResource>(strResourcePath);
		if (!pResource)
		{
			MORTY_ASSERT(pResource);
			return nullptr;
		}

		auto pTextureData = pResourceSystem->LoadResourceData(strFullPath);
		

		if (eUsage == MEModelTextureUsage::Metallic)
		{
			MTextureConverter::ConvertSingleChannel(static_cast<MTextureResourceData*>(pTextureData.get()), 2);
		}
		else if (eUsage == MEModelTextureUsage::Roughness)
		{	
			MTextureConverter::ConvertSingleChannel(static_cast<MTextureResourceData*>(pTextureData.get()), 1);
		}

		pResource->Load(std::move(pTextureData));
		std::shared_ptr<MTextureResource> pTexture = MTypeClass::DynamicCast<MTextureResource>(pResource);

		m_tTextures[strResourcePath] = pTexture;

		return pTexture;
	}
	

private:

	std::map<MString, std::shared_ptr<MTextureResource> > m_tTextures;

	MEngine* m_pEngine = nullptr;
};

void LOAD_MODEL_SPONZA_TEST(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = pEngine->FindSystem<MEntitySystem>();


	auto time = MTimer::GetCurTime();

	std::shared_ptr<MResource> pModelResource = pResourceSystem->LoadResource("./Sponza/Sponza/Sponza.entity");

	time = MTimer::GetCurTime() - time;

	pEngine->GetLogger()->Log("Load sponza scene time: {}", time);

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
			pEntitySystem->FindAllComponentRecursively(pEntity, MRenderMeshComponent::GetClassType(), vMeshComponents);
			
		}

		if (vEntity.size() > 0)
		{
			vEntity[0]->GetComponent<MSceneComponent>()->SetScale({ 10.0,10.0, 10.0 });
		}
	}

	std::set<std::shared_ptr<MMaterial>> tMaterials;

	for (MComponentID& componentID : vMeshComponents)
	{
		if (MRenderMeshComponent* pMeshComponent = pScene->GetComponent(componentID)->template DynamicCast<MRenderMeshComponent>())
		{
			if (auto pMaterial = pMeshComponent->GetMaterial())
			{
				tMaterials.insert(pMaterial);
			}

			pMeshComponent->SetGenerateDirLightShadow(true);
		}

	}
	
}
