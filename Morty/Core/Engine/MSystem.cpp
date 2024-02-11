#include "Engine/MSystem.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MISystem, MTypeClass)

MISystem::MISystem()
	: m_pEngine(nullptr)
{

}

MISystem::~MISystem()
{

}

void MISystem::SetEngine(MEngine* pEngine)
{
	m_pEngine = pEngine;
}

MEngine* MISystem::GetEngine()
{
	return m_pEngine;
}
