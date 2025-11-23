#include "Object/MObject.h"
#include "Engine/MEngine.h"
#include "System/MObjectSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MObject, MTypeClass)

MObjectSystem* MObject::GetObjectSystem()
{
    if (nullptr == m_engine) return nullptr;

    if (MISystem* pSystem = m_engine->FindSystem(MObjectSystem::GetClassType()))
    {
        return pSystem->template DynamicCast<MObjectSystem>();
    }

    return nullptr;
}

void MObject::DeleteLater()
{
    if (!m_deleteMark)
    {
        m_deleteMark = true;

        GetObjectSystem()->RemoveObject(m_objectID);
    }
}
