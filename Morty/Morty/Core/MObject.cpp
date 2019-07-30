#include "MObject.h"

MObject::MObject()
	: m_unObjectID(0)
{

}

MObject::~MObject()
{

}

MObjectManager::MObjectManager()
	: m_pObjectDB(new MObjectDB())
{

}

MObjectManager::~MObjectManager()
{
	delete m_pObjectDB;
}

MObjectDB::MObjectDB()
	: m_unObjectIDPool(0)
{
	
}

MObjectID MObjectDB::GetNewID()
{
	return ++m_unObjectIDPool;
}
