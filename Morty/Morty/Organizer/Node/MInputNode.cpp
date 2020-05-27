#include "MInputNode.h"

M_OBJECT_IMPLEMENT(MInputNode, MNode)

MInputNode::MInputNode()
    : MNode()
	, m_funcInputCallback(nullptr)
{
}

MInputNode::~MInputNode()
{
}

bool MInputNode::Input(MInputEvent* pEvent, MViewport* pViewport)
{
	if (m_funcInputCallback)
		return m_funcInputCallback(pEvent, pViewport);

	return false;
}

