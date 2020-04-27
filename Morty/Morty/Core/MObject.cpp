#include "MObject.h"
#include "MEngine.h"

MTypeIdentifierImplement(MObject, MTypedClass)

MObject::MObject()
	: m_unObjectID(0)
	, m_pEngine(nullptr)
	, m_bDeleteMark(false)
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

void MObject::DeleteLater()
{
	if (!m_bDeleteMark)
	{
		m_bDeleteMark = true;
		m_pEngine->GetObjectManager()->RemoveObject(m_unObjectID);
	}
}

MObjectManager::MObjectManager()
	: m_pObjectDB(new MIDPool<MObjectID>())
	, m_pEngine(nullptr)
{

}

MObjectManager::~MObjectManager()
{
	for (std::map<MObjectID, MObject*>::iterator iter = m_tObjects.begin(); iter != m_tObjects.end(); ++iter)
	{
		delete iter->second;
	}

	m_tObjects.clear();

	delete m_pObjectDB;
}

void MObjectManager::SetOwnerEngine(MEngine* pEngine)
{
	m_pEngine = pEngine;
}
