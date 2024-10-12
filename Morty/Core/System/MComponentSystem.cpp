#include "System/MComponentSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MComponentSystem, MISystem)

MComponentSystem::MComponentSystem()
    : MISystem()
    , m_componentGroupFactory()
{}

MComponentSystem::~MComponentSystem() {}

MIComponentGroup* MComponentSystem::CreateComponentGroup(const MType* pComponentType)
{
    auto findResult = m_componentGroupFactory.find(pComponentType);

    if (findResult == m_componentGroupFactory.end()) return nullptr;

    return findResult->second();
}
