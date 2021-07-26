﻿#include "MObject.h"
#include "MEngine.h"
#include "MObjectSystem.h"

MORTY_CLASS_IMPLEMENT(MObject, MTypeClass)

MObject::MObject()
	: m_unObjectID(0)
	, m_pEngine(nullptr)
	, m_bDeleteMark(false)
	, m_vPointer()
{

}

MObject::~MObject()
{

}

class MObjectSystem* MObject::GetObjectSystem()
{
	if (nullptr == m_pEngine)
		return nullptr;

	if (MISystem* pSystem = m_pEngine->FindSystem(MObjectSystem::GetClassType()))
	{
		return pSystem->DynamicCast<MObjectSystem>();
	}

	return nullptr;
}

void MObject::DeleteLater()
{
	if (!m_bDeleteMark)
	{
		m_bDeleteMark = true;

		GetObjectSystem()->RemoveObject(m_unObjectID);
	}
}

