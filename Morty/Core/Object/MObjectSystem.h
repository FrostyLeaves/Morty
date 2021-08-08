/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       DoubleYe
**/

#ifndef _M_MOBJECT_SYSTEM_H_
#define _M_MOBJECT_SYSTEM_H_
#include "MGlobal.h"

#include "MType.h"
#include "MIDPool.h"
#include "MString.h"
#include "MObject.h"
#include "MSystem.h"

#include <map>
#include <vector>

class MEngine;
class MORTY_API MObjectSystem : public MISystem
{
public:
	MORTY_CLASS(MObjectSystem)
public:
	MObjectSystem();
	virtual ~MObjectSystem();

	virtual void Tick(const float& fDelta) {}

	void InitObject(MObject* pObject);

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

	MObject* CreateObject(const MString& strTypeName);

	MObject* FindObject(const MObjectID& unID);

	void RemoveObject(const MObjectID& unID);

	void CleanRemoveObject();

public:

	virtual void Release() override;

private:
	MIDPool<MObjectID>* m_pObjectDB;

	std::map<MObjectID, MObject*> m_tObjects;

	std::vector<MObjectID> m_vRemoveObjects;
};

#endif
