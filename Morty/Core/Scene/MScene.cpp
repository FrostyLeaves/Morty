#include "Scene/MScene.h"
#include "Component/MComponent.h"
#include "Engine/MEngine.h"
#include "Utility/MFunction.h"

#include "System/MComponentSystem.h"

#include "TaskGraph/MTaskGraph.h"
#include "TaskGraph/MTaskNode.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MScene, MObject)

MScene::MScene()
    : MObject()
    , m_components()
{}

MScene::~MScene() {}

MEntity* MScene::CreateEntity()
{
    MGuid id = MGuid::generate();
    return CreateEntity(id);
}

MEntity* MScene::CreateEntity(const MGuid& guid)
{
    MEntity* pEntity = new MEntity(this, guid);
    m_entity[guid]   = pEntity;

    return pEntity;
}

void MScene::DeleteEntity(MEntity* pEntity)
{
    //Delete Components.
    if (!pEntity) return;

    pEntity->UnregisterAllComponent();

    auto findResult = m_entity.find(pEntity->GetID());
    if (findResult != m_entity.end()) { m_entity.erase(findResult); }
}

MEntity* MScene::GetEntity(const MGuid& id)
{
    auto findResult = m_entity.find(id);
    if (findResult != m_entity.end()) return findResult->second;

    return nullptr;
}

void MScene::OnCreated() { MObject::OnCreated(); }

void MScene::OnDelete()
{
    while (!m_manager.empty())
    {
        (m_manager.begin()->second)->Release();
        m_manager.erase(m_manager.begin());
    }

    for (auto pr: m_components) { pr.second->RemoveAllComponents(); }
    m_components.clear();

    m_entity.clear();

    Super::OnDelete();
}

MEntity* MScene::FindFirstEntityByComponent(const MType* pComponentType)
{
    MIComponentGroup* pGroup = FindComponents(pComponentType);

    if (!pGroup) return nullptr;

    MComponent* pComponent = pGroup->FirstComponent();
    if (!pComponent) return nullptr;

    return pComponent->GetEntity();
}

MIComponentGroup* MScene::FindComponents(const MType* pComponentType)
{
    auto findResult = m_components.find(pComponentType);
    if (findResult == m_components.end())
    {
        if (MIComponentGroup* pGroup = CreateComponents(pComponentType))
        {
            return m_components[pComponentType] = pGroup;
        }

        return nullptr;
    }

    return findResult->second;
}

MComponent* MScene::FindComponent(MEntity* entity, const MType* pComponentType)
{
    auto findResult = entity->m_components.find(pComponentType);
    if (findResult == entity->m_components.end()) return nullptr;

    return findResult->second;

    return nullptr;
}

MComponent* MScene::GetComponent(const MComponentID& id)
{
    if (!id.IsValid()) return nullptr;

    if (MIComponentGroup* pComponents = FindComponents(id.pComponentType))
    {
        return pComponents->FindComponent(id);
    }

    return nullptr;
}

std::vector<MEntity*> MScene::GetAllEntity() const
{
    std::vector<MEntity*> result;
    for (const auto& pr: m_entity) { result.push_back(pr.second); }
    return result;
}

void MScene::RegisterManager(const MType* pManagerType, IManager* pManager)
{
    m_manager[pManagerType] = pManager;
    pManager->SetScene(this);

    for (const MType* type: pManager->RegisterComponentType())
    {
        m_componentRegister[type].push_back(pManager);
    }
}

void MScene::Tick(const float& fDelta)
{
    MEngine* pEngine = GetEngine();
    if (!pEngine) return;

    auto& vSystem = pEngine->GetAllSystem();

    for (MISystem* pSystem: vSystem) { pSystem->SceneTick(this, fDelta); }

    for (auto pr: m_manager) { pr.second->SceneTick(this, fDelta); }
}

MComponent* MScene::AddComponent(MEntity* entity, MIComponentGroup* pComponents)
{
    if (!entity) return nullptr;

    MComponentID compID = pComponents->AddComponent(entity);

    MComponent*  pComponent                     = pComponents->FindComponent(compID);
    entity->m_components[compID.pComponentType] = pComponent;

    auto findRegister = m_componentRegister.find(pComponent->GetType());
    if (findRegister != m_componentRegister.end())
    {
        for (IManager* pManager: findRegister->second)
        {
            pManager->RegisterComponent(pComponent);
        }
    }

    return pComponent;
}

MComponent* MScene::AddComponent(MEntity* entity, const MType* pComponentType)
{
    if (MIComponentGroup* pComponents = FindComponents(pComponentType))
    {
        return AddComponent(entity, pComponents);
    }

    return nullptr;
}

MIComponentGroup* MScene::CreateComponents(const MType* pComponentType)
{
    if (MComponentSystem* pComponentSystem = GetEngine()->FindSystem<MComponentSystem>())
    {
        MIComponentGroup* pGroup = pComponentSystem->CreateComponentGroup(pComponentType);
        if (pGroup) { pGroup->m_scene = this; }
        return pGroup;
    }

    return nullptr;
}

void MScene::RemoveComponent(MEntity* entity, const MType* pComponentType)
{
    if (!entity) return;

    MIComponentGroup* pComponents = FindComponents(pComponentType);
    if (!pComponents) return;

    auto findResult = entity->m_components.find(pComponentType);
    if (findResult == entity->m_components.end()) { return; }

    MComponent* pComponent = findResult->second;

    entity->m_components.erase(findResult);
    if (!pComponent)
    {
        MORTY_ASSERT(pComponent);
        return;
    }

    auto findRegister = m_componentRegister.find(pComponentType);
    if (findRegister != m_componentRegister.end())
    {
        for (IManager* pManager: findRegister->second)
        {
            pManager->UnregisterComponent(pComponent);
        }
    }
    pComponents->RemoveComponent(pComponent->GetComponentID());
}
