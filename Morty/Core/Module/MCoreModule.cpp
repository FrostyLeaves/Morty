#include "Module/MCoreModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"

#include "System/MComponentSystem.h"
#include "System/MEntitySystem.h"
#include "System/MInputSystem.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MInputComponent.h"
#include "Component/MSceneComponent.h"

#include "Resource/MEntityResource.h"
#include "Resource/MResourceAsyncLoadSystem.h"

using namespace morty;

bool MCoreModule::Register(MEngine* pEngine)
{
    if (!pEngine) return false;

    pEngine->RegisterSystem<MInputSystem>();
    ;
    pEngine->RegisterSystem<MEntitySystem>();


    if (MObjectSystem* pObjectSystem = pEngine->RegisterSystem<MObjectSystem>())
    {
        pObjectSystem->RegisterPostCreateObject(MCoreModule::OnObjectPostCreate);
    }

    if (MResourceSystem* pResourceSystem = pEngine->RegisterSystem<MResourceSystem>())
    {
#ifdef MORTY_RESOURCE_PATH
        pResourceSystem->SetSearchPath({MORTY_RESOURCE_PATH});
#endif
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
    if (!pObject) { return; }

    if (pObject->GetType() == MScene::GetClassType())
    {
        if (MScene* pScene = pObject->template DynamicCast<MScene>())
        {
            pScene->RegisterManager<MNotifyManager>();
        }
    }
}
