#include "Engine/MSystem.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MISystem, MTypeClass)

MISystem::MISystem()
    : m_engine(nullptr)
{}

MISystem::~MISystem() {}

void     MISystem::SetEngine(MEngine* pEngine) { m_engine = pEngine; }

MEngine* MISystem::GetEngine() { return m_engine; }
