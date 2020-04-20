#include "MIMeshInstance.h"

MTypeIdentifierImplement(MIMeshInstance, M3DNode)

MIMeshInstance::MIMeshInstance()
	: M3DNode()
	, m_eRenderOrderType(MERenderOrderType::EAutoOrder)
{

}

MIMeshInstance::~MIMeshInstance()
{

}
