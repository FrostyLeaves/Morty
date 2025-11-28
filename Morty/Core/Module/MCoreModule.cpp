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

bool MCoreModule::Register(MEngine* engine)
{
    if (!engine) return false;

    engine->RegisterSystem<MInputSystem>();
    ;
    engine->RegisterSystem<MEntitySystem>();


    if (auto* objectSystem = engine->RegisterSystem<MObjectSystem>())
    {
        objectSystem->RegisterPostCreateObject(MCoreModule::OnObjectPostCreate);
    }

    if (auto* resourceSystem = engine->RegisterSystem<MResourceSystem>())
    {
#ifdef MORTY_RESOURCE_PATH
        resourceSystem->SetSearchPath({MORTY_RESOURCE_PATH});
#endif
        resourceSystem->RegisterResourceLoader<MEntityResourceLoader>();
    }

    engine->RegisterSystem<MResourceAsyncLoadSystem>();

    if (auto* pComponentSystem = engine->RegisterSystem<MComponentSystem>())
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
        if (auto* scene = pObject->template DynamicCast<MScene>()) { scene->RegisterManager<MNotifyManager>(); }
    }
}
