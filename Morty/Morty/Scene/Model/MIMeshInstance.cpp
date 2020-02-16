#include "MIMeshInstance.h"

MTypeIdentifierImplement(MIMeshInstance, M3DNode)

MIMeshInstance::MIMeshInstance()
	: M3DNode()
	, m_pModelInstance(nullptr)
	, m_eShadowType(MEShadowType::EOnlyDirectional)
{

}

MIMeshInstance::~MIMeshInstance()
{

}
