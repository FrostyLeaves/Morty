#include "MCoreModule.h"
#include "MEngine.h"
#include "MEntity.h"

#include "MInputSystem.h"
#include "MNotifySystem.h"
#include "MObjectSystem.h"
#include "MEntitySystem.h"
#include "MResourceSystem.h"
#include "MComponentSystem.h"

#include "MSceneComponent.h"
#include "MInputComponent.h"
#include "MNotifyComponent.h"

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
		pComponentSystem->RegisterComponent<MNotifyComponent>();
	}

	return true;
}
