#include "Module/MCoreModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"

#include "System/MInputSystem.h"
#include "System/MNotifySystem.h"
#include "System/MObjectSystem.h"
#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "System/MComponentSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MInputComponent.h"

bool MCoreModule::Register(MEngine* pEngine)
{
	if (!pEngine)
		return false;
	
	pEngine->RegisterSystem<MInputSystem>();
	pEngine->RegisterSystem<MNotifySystem>();
	pEngine->RegisterSystem<MObjectSystem>();
	pEngine->RegisterSystem<MEntitySystem>();

	if (MResourceSystem* pResourceSystem = pEngine->RegisterSystem<MResourceSystem>())
	{
		pResourceSystem->SetSearchPath({ MORTY_RESOURCE_PATH });
	}

	if (MComponentSystem* pComponentSystem = pEngine->RegisterSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MSceneComponent>();
		pComponentSystem->RegisterComponent<MInputComponent>();
	}

	return true;
}
