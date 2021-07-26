﻿#include "MObjectSystem.h"
#include "MEngine.h"

MORTY_CLASS_IMPLEMENT(MObjectSystem, MISystem)

MObjectSystem::MObjectSystem()
	: m_pObjectDB(new MIDPool<MObjectID>())
{
}

MObjectSystem::~MObjectSystem()
{
	for (std::map<MObjectID, MObject*>::iterator iter = m_tObjects.begin(); iter != m_tObjects.end(); ++iter)
	{
		delete iter->second;
		iter->second = nullptr;
	}

	m_tObjects.clear();

	delete m_pObjectDB;
}

void MObjectSystem::InitObject(MObject* pObject)
{
	pObject->m_unObjectID = m_pObjectDB->GetNewID();
	pObject->m_pEngine = GetEngine();

	m_tObjects[pObject->m_unObjectID] = pObject;

	pObject->OnCreated();
}

MObject* MObjectSystem::CreateObject(const MString& strTypeName)
{
	if (MTypeClass* pTypedIns = MTypeClass::New(strTypeName))
	{
		if (MObject* pObject = pTypedIns->DynamicCast<MObject>())
		{
			InitObject(pObject);
			return pObject;
		}
	}

	return nullptr;
}

MObject* MObjectSystem::FindObject(const MObjectID& unID)
{
	std::map<MObjectID, MObject*>::iterator iter = m_tObjects.find(unID);
	if (iter == m_tObjects.end())
		return nullptr;

	return iter->second;
}

void MObjectSystem::RemoveObject(const MObjectID& unID)
{
	m_vRemoveObjects.push_back(unID);
}

void MObjectSystem::CleanRemoveObject()
{
	while (!m_vRemoveObjects.empty())
	{
		const MObjectID& unID = m_vRemoveObjects.back();
		m_vRemoveObjects.pop_back();

		std::map<MObjectID, MObject*>::iterator iter = m_tObjects.find(unID);
		if (iter != m_tObjects.end())
		{
			iter->second->OnDelete();
			delete iter->second;
			m_tObjects.erase(iter);
		}
	}

}
