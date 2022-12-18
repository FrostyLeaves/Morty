#include "MRenderModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Utility/MFunction.h"

#include "System/MModelSystem.h"
#include "System/MNotifySystem.h"
#include "System/MRenderSystem.h"
#include "System/MSkyBoxSystem.h"
#include "System/MResourceSystem.h"
#include "System/MComponentSystem.h"

#include "Resource/MMeshResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MEntityResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"

#include "Component/MModelComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MDebugRenderComponent.h"
#include "MergeInstancing/MMergeInstancingSubSystem.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"

#include "TaskGraph/MTaskGraph.h"

const MString MRenderModule::DefaultWhite = "Default_White";
const MString MRenderModule::DefaultNormal = "Default_Normal";
const MString MRenderModule::Default_R8_One = "Default_R8_One";
const MString MRenderModule::Default_R8_Zero = "Default_R8_Zero";

bool MRenderModule::Register(MEngine* pEngine)
{
	if (!pEngine)
		return false;

	MNotifySystem* pNotifySystem = pEngine->FindSystem<MNotifySystem>();

	pEngine->RegisterSystem<MModelSystem>();

	if (MRenderSystem* pRenderSystem = pEngine->RegisterSystem<MRenderSystem>())
	{
		pNotifySystem->RegisterNotify("TransformDirty", M_CLASS_FUNCTION_BIND_0_1(MRenderSystem::OnTransformDirty, pRenderSystem));

		if (MTaskGraph* pTaskGraph = pEngine->GetMainGraph())
		{
			if (MTaskNode* pTaskNode = pTaskGraph->AddNode<MTaskNode>("Render_Update"))
			{
				pTaskNode->SetThreadType(METhreadType::ERenderThread);
				pTaskNode->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MRenderSystem::Update, pRenderSystem));
			}
		}
	}

	MSkyBoxSystem* pSkyBoxSystem = pEngine->RegisterSystem<MSkyBoxSystem>();

	if (MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>())
	{
		pResourceSystem->RegisterResourceType<MMeshResource>();
		pResourceSystem->RegisterResourceType<MShaderResource>();
		pResourceSystem->RegisterResourceType<MEntityResource>();
		pResourceSystem->RegisterResourceType<MTextureResource>();
		pResourceSystem->RegisterResourceType<MMaterialResource>();
		pResourceSystem->RegisterResourceType<MSkeletonResource>();
		pResourceSystem->RegisterResourceType<MSkeletalAnimationResource>();


		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(DefaultWhite))
		{
			MByte byte[4];
			byte[0] = byte[1] = byte[2] = byte[3] = 255;
			pTexture->LoadFromMemory(byte, 1, 1, 4);
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(DefaultNormal))
		{
			MByte byte[3];
			byte[0] = byte[1] = 127;
			byte[2] = 255;
			pTexture->LoadFromMemory(byte, 1, 1, 3);
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(Default_R8_One))
		{
			MByte byte = 255;
			pTexture->LoadFromMemory(&byte, 1, 1, 1);
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(Default_R8_Zero))
		{
			MByte byte = 0;
			pTexture->LoadFromMemory(&byte, 1, 1, 1);
		}
	}

	if (MComponentSystem* pComponentSystem = pEngine->FindSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MModelComponent>();
		pComponentSystem->RegisterComponent<MCameraComponent>();
		pComponentSystem->RegisterComponent<MSpotLightComponent>();
		pComponentSystem->RegisterComponent<MPointLightComponent>();
		pComponentSystem->RegisterComponent<MRenderableMeshComponent>();
		pComponentSystem->RegisterComponent<MDirectionalLightComponent>();
		pComponentSystem->RegisterComponent<MDebugRenderComponent>();
	}


	if (MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>())
	{
		pObjectSystem->RegisterPostCreateObject(MRenderModule::OnObjectPostCreate);
	}
	

	return true;
}

void MRenderModule::OnObjectPostCreate(MObject* pObject)
{
	if (!pObject)
	{
		return;
	}

	if(pObject->GetType() == MScene::GetClassType())
	{
		if (MScene* pScene = pObject->DynamicCast<MScene>())
		{
			pScene->RegisterSubSystem<MMergeInstancingSubSystem>();
		}
	}
}
