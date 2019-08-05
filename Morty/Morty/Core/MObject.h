/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       Morty
**/

#ifndef _M_MOBJECT_H_
#define _M_MOBJECT_H_
#include "MGlobal.h"

#include <map>

class MORTY_CLASS MObject
{
public:
    MObject();
    virtual ~MObject();

public:

	MObjectID GetObjectID(){ return m_unObjectID; }

private:

	friend class MObjectManager;

	MObjectID m_unObjectID;
};

class MObjectDB
{
public:

	MObjectDB();

	MObjectID GetNewID();

private:
	MObjectID m_unObjectIDPool;
};

class MObjectManager
{
public:
	MObjectManager();
	virtual ~MObjectManager();

	template<typename Object_TYPE>
	Object_TYPE* CreateObject()
	{
		Object_TYPE* pObject = new Object_TYPE();
		if (!dynamic_cast<MObject*>(pObject))
		{
			delete pObject;
			return nullptr;
		}

		pObject->m_unObjectID = m_pObjectDB->GetNewID();

		m_tObjects[pObject->m_unObjectID] = pObject;

		return pObject;
	}

	MObject* FindObject(const MObjectID& unID)
	{
		std::map<MObjectID, MObject*>::iterator iter = m_tObjects.find(unID);
		if (iter == m_tObjects.end())
			return nullptr;

		return iter->second;
	}

	void RemoveObject(const MObjectID& unID)
	{
		std::map<MObjectID, MObject*>::iterator iter = m_tObjects.find(unID);
		if (iter != m_tObjects.end())
		{
			m_tObjects.erase(iter);
		}
	}

private:
	MObjectDB* m_pObjectDB;


	std::map<MObjectID, MObject*> m_tObjects;
};

#endif
