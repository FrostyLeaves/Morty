#include "MObject.h"
#include "MEngine.h"

MObject::MObject()
	: m_unObjectID(0)
	, m_pEngine(nullptr)
{

}

MObject::~MObject()
{

}

class MObjectManager* MObject::GetObjectManager()
{
	if (nullptr == m_pEngine)
		return nullptr;

	return m_pEngine->GetObjectManager();
}

MObjectManager::MObjectManager()
	: m_pObjectDB(new MIDPool<MObjectID>())
	, m_pEngine(nullptr)
{

}

MObjectManager::~MObjectManager()
{
	delete m_pObjectDB;
}

void MObjectManager::SetOwnerEngine(MEngine* pEngine)
{
	m_pEngine = pEngine;
}
