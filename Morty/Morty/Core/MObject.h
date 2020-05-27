/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       Pobrecito
**/

#ifndef _M_MOBJECT_H_
#define _M_MOBJECT_H_
#include "MGlobal.h"
#include "MIDPool.h"
#include "MString.h"
#include "MTypedClass.h"

#include <vector>
#include <map>

#define M_I_OBJECT(Class) \
MTypedInterfaceSign(Class)

#define M_OBJECT(Class) \
MTypedClassSign(Class);

#define M_I_OBJECT_IMPLEMENT(Class, BaseClass) \
MTypedInterfaceImplement(Class, BaseClass)

#define M_OBJECT_IMPLEMENT(Class, BaseClass)\
MTypedClassImplement(Class, BaseClass)

class MEngine;
class MObjectManager;
class MORTY_CLASS MObject : public MTypedClass
{
public:
	M_OBJECT(MObject);
    MObject();
    virtual ~MObject();

public:

	MObjectID GetObjectID(){ return m_unObjectID; }
	MEngine* GetEngine() { return m_pEngine; }
	MObjectManager* GetObjectManager();


	void DeleteLater();

public:
	virtual void OnCreated() {};
	virtual void OnDelete() {};

protected:

	friend class MObjectManager;

	MObjectID	m_unObjectID;
	MEngine*	m_pEngine;
	bool		m_bDeleteMark;
};

class MORTY_CLASS MObjectManager
{
public:
	MObjectManager();
	virtual ~MObjectManager();

	void SetOwnerEngine(MEngine* pEngine);

	void InitObject(MObject* pObject)
	{
		pObject->m_unObjectID = m_pObjectDB->GetNewID();
		pObject->m_pEngine = m_pEngine;

		m_tObjects[pObject->m_unObjectID] = pObject;

		pObject->OnCreated();

	}

	template<typename Object_TYPE>
	Object_TYPE* CreateObject()
	{
		Object_TYPE* pObject = new Object_TYPE();
		if (!dynamic_cast<MObject*>(pObject))
		{
			delete pObject;
			return nullptr;
		}

		InitObject(pObject);
		return pObject;
	}

	MObject* CreateObject(const MString& strTypeName)
	{
		if (MTypedClass* pTypedIns = MTypedClass::New(strTypeName))
		{
			if (MObject* pObject = pTypedIns->DynamicCast<MObject>())
			{
				InitObject(pObject);
				return pObject;
			}
		}

		return nullptr;
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
		m_vRemoveObjects.push_back(unID);
	}

	void CleanRemoveObject();

private:
	MIDPool<MObjectID>* m_pObjectDB;

	std::map<MObjectID, MObject*> m_tObjects;

	MEngine* m_pEngine;

	std::vector<MObjectID> m_vRemoveObjects;
};

#endif
