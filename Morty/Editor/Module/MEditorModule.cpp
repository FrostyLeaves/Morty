#include "MEditorModule.h"
#include "MEngine.h"
#include "MEntity.h"

#include "MComponentSystem.h"
#include "MMoveControllerSystem.h"

#include "MMoveControllerComponent.h"

bool MEditorModule::Register(MEngine* pEngine)
{
	if (!pEngine)
		return false;


	pEngine->RegisterSystem<MMoveControllerSystem>();
	
	if (MComponentSystem* pComponentSystem = pEngine->FindSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MMoveControllerComponent>();
	}

	return true;
}