#include "MComponentSystem.h"

MORTY_CLASS_IMPLEMENT(MComponentSystem, MISystem)

MComponentSystem::MComponentSystem()
	: MISystem()
	, m_tComponentGroupFactory()
{

}

MComponentSystem::~MComponentSystem()
{

}

MIComponentGroup* MComponentSystem::CreateComponentGroup(const MType* pComponentType)
{
	auto findResult = m_tComponentGroupFactory.find(pComponentType);

	if (findResult == m_tComponentGroupFactory.end())
		return nullptr;

	return findResult->second();
}
