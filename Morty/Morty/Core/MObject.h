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
#include "MIDPool.h"

#include <map>

class MEngine;
class MORTY_CLASS MObject
{
public:
    MObject();
    virtual ~MObject();

public:

	MObjectID GetObjectID(){ return m_unObjectID; }
	class MObjectManager* GetObjectManager();

public:
	virtual void OnCreated() {};

protected:

	friend class MObjectManager;

	MObjectID m_unObjectID;
	MEngine* m_pEngine;
};

class MORTY_CLASS MObjectManager
{
public:
	MObjectManager();
	virtual ~MObjectManager();

	void SetOwnerEngine(MEngine* pEngine);

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
		pObject->m_pEngine = m_pEngine;

		m_tObjects[pObject->m_unObjectID] = pObject;

		pObject->OnCreated();

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
			delete iter->second;
			m_tObjects.erase(iter);
		}
	}

private:
	MIDPool<MObjectID>* m_pObjectDB;

	std::map<MObjectID, MObject*> m_tObjects;

	MEngine* m_pEngine;
};

#endif
