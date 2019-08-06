#include "MObject.h"

MObject::MObject()
	: m_unObjectID(0)
{

}

MObject::~MObject()
{

}

MObjectManager::MObjectManager()
	: m_pObjectDB(new MIDPool<MObjectID>())
{

}

MObjectManager::~MObjectManager()
{
	delete m_pObjectDB;
}
