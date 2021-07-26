/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       DoubleYe
**/

#ifndef _M_MOBJECT_H_
#define _M_MOBJECT_H_
#include "MGlobal.h"

#include "MType.h"
#include "MIDPool.h"
#include "MString.h"

#include <vector>
#include <map>

class MEngine;
class MAutoPtr;
class MObjectSystem;
class MORTY_API MObject : public MTypeClass
{
public:
	MORTY_CLASS(MObject);

    MObject();
    virtual ~MObject();

public:

	MObjectID GetObjectID(){ return m_unObjectID; }
	MEngine* GetEngine() { return m_pEngine; }
	MObjectSystem* GetObjectSystem();


	void DeleteLater();
	bool GetDeleteMark() { return m_bDeleteMark; }

public:
	virtual void OnCreated() {};
	virtual void OnDelete() {};

protected:

	friend class MObjectSystem;

	MObjectID	m_unObjectID;
	MEngine*	m_pEngine;
	
private:
	bool		m_bDeleteMark;

	std::vector<MAutoPtr*> m_vPointer;
};


#endif
