#include "Module/MCoreModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"

#include "System/MInputSystem.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "System/MComponentSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MInputComponent.h"

#include "Resource/MEntityResource.h"
#include "Resource/MResourceAsyncLoadSystem.h"

bool MCoreModule::Register(MEngine* pEngine)
{
	if (!pEngine)
		return false;
	
	pEngine->RegisterSystem<MInputSystem>();
	;
	pEngine->RegisterSystem<MEntitySystem>();


	if (MObjectSystem* pObjectSystem = pEngine->RegisterSystem<MObjectSystem>())
	{
		pObjectSystem->RegisterPostCreateObject(MCoreModule::OnObjectPostCreate);
	}

	if (MResourceSystem* pResourceSystem = pEngine->RegisterSystem<MResourceSystem>())
	{
		pResourceSystem->SetSearchPath({ MORTY_RESOURCE_PATH });

		pResourceSystem->RegisterResourceLoader<MEntityResourceLoader>();
	}

	pEngine->RegisterSystem<MResourceAsyncLoadSystem>();

	if (MComponentSystem* pComponentSystem = pEngine->RegisterSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MSceneComponent>();
		pComponentSystem->RegisterComponent<MInputComponent>();
	}

	return true;
}

void MCoreModule::OnObjectPostCreate(MObject* pObject)
{
	if (!pObject)
	{
		return;
	}

	if (pObject->GetType() == MScene::GetClassType())
	{
		if (MScene* pScene = pObject->DynamicCast<MScene>())
		{
			pScene->RegisterManager<MNotifyManager>();
		}
	}
}
