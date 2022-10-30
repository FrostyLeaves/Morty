﻿/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       DoubleYe
**/

#ifndef _M_MOBJECT_SYSTEM_H_
#define _M_MOBJECT_SYSTEM_H_
#include "Utility/MGlobal.h"

#include "Type/MType.h"
#include "Utility/MIDPool.h"
#include "Utility/MString.h"
#include "Object/MObject.h"
#include "Engine/MSystem.h"

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

	template<typename OBJECT_TYPE>
	OBJECT_TYPE* CreateObject()
	{
		OBJECT_TYPE* pObject = new OBJECT_TYPE();
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
