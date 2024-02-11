#include "Object/MObject.h"
#include "Engine/MEngine.h"
#include "System/MObjectSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MObject, MTypeClass)

MObject::MObject()
	: m_unObjectID(0)
	, m_pEngine(nullptr)
	, m_bDeleteMark(false)
{

}

MObject::~MObject()
{

}

class MObjectSystem* MObject::GetObjectSystem()
{
	if (nullptr == m_pEngine)
		return nullptr;

	if (MISystem* pSystem = m_pEngine->FindSystem(MObjectSystem::GetClassType()))
	{
		return pSystem->template DynamicCast<MObjectSystem>();
	}

	return nullptr;
}

void MObject::DeleteLater()
{
	if (!m_bDeleteMark)
	{
		m_bDeleteMark = true;

		GetObjectSystem()->RemoveObject(m_unObjectID);
	}
}

