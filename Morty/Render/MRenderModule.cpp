#include "MRenderModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Utility/MFunction.h"

#include "System/MModelSystem.h"
#include "System/MNotifyManager.h"
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
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MDebugRenderComponent.h"
#include "Component/MSkyBoxComponent.h"

#include "Shadow/MShadowMeshManager.h"
#include "Manager/MEnvironmentManager.h"

#include "Batch/MMeshInstanceManager.h"
#include "Manager/MAnimationManager.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"

#include "Mesh/MMeshManager.h"

#include "TaskGraph/MTaskGraph.h"

#include "Module/MCoreNotify.h"
#include "Resource/MMaterialResourceData.h"
#include "Resource/MTextureResourceUtil.h"

const MString MRenderModule::DefaultWhite = "Default_White";
const MString MRenderModule::DefaultNormal = "Default_Normal";
const MString MRenderModule::Default_R8_One = "Default_R8_One";
const MString MRenderModule::Default_R8_Zero = "Default_R8_Zero";
const MString MRenderModule::DefaultAnimationMaterial = "Default_Animation_Material";

bool MRenderModule::Register(MEngine* pEngine)
{
	if (!pEngine)
	{
		return false;
	}

	MTaskGraph* pTaskGraph = pEngine->GetMainGraph();

	pEngine->RegisterSystem<MModelSystem>();
	pEngine->RegisterSystem<MSkyBoxSystem>();

	MRenderSystem* pRenderSystem = pEngine->RegisterSystem<MRenderSystem>();

	if (MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>())
	{
		pResourceSystem->RegisterResourceLoader<MMeshResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MShaderResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MTextureResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MMaterialResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MSkeletonResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MSkeletalAnimationLoader>();


		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(DefaultWhite))
		{
			MByte byte[4];
			byte[0] = byte[1] = byte[2] = byte[3] = 255;
 			pTexture->Load(MTextureResourceUtil::LoadFromMemory("Default_White", byte, 1, 1, 4));
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(DefaultNormal))
		{
			MByte byte[3];
			byte[0] = byte[1] = 127;
			byte[2] = 255;
			pTexture->Load(MTextureResourceUtil::LoadFromMemory("Default_Normal", byte, 1, 1, 3));
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(Default_R8_One))
		{
			MByte byte = 255;
			pTexture->Load(MTextureResourceUtil::LoadFromMemory("R8_One", & byte, 1, 1, 1));
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(Default_R8_Zero))
		{
			MByte byte = 0;
			pTexture->Load(MTextureResourceUtil::LoadFromMemory("R8_Zero", & byte, 1, 1, 1));
		}
	}

	pEngine->RegisterGlobalObject<MMeshManager>();

	if (MComponentSystem* pComponentSystem = pEngine->FindSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MModelComponent>();
		pComponentSystem->RegisterComponent<MCameraComponent>();
		pComponentSystem->RegisterComponent<MSpotLightComponent>();
		pComponentSystem->RegisterComponent<MPointLightComponent>();
		pComponentSystem->RegisterComponent<MRenderMeshComponent>();
		pComponentSystem->RegisterComponent<MDirectionalLightComponent>();
		pComponentSystem->RegisterComponent<MDebugRenderComponent>();
		pComponentSystem->RegisterComponent<MSkyBoxComponent>();
	}


	if (MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>())
	{
		pObjectSystem->RegisterPostCreateObject(MRenderModule::OnObjectPostCreate);
	}

	MTaskNode* pRenderUpdateTask = pTaskGraph->AddNode<MTaskNode>(MRenderGlobal::TASK_RENDER_MODULE_UPDATE);
	pRenderUpdateTask->SetThreadType(METhreadType::ERenderThread);
	pRenderUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MRenderSystem::Update, pRenderSystem));



	


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
		if (MScene* pScene = pObject->template DynamicCast<MScene>())
		{
			pScene->RegisterManager<MMeshInstanceManager>();
			pScene->RegisterManager<MEnvironmentManager>();
			pScene->RegisterManager<MShadowMeshManager>();
			pScene->RegisterManager<MAnimationManager>();
		}
	}
}
