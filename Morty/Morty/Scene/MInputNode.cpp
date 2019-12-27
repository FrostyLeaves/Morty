#include "MInputNode.h"

MTypeIdentifierImplement(MInputNode, MNode)

MInputNode::MInputNode()
    : MNode()
	, m_funcInputCallback(nullptr)
{
}

MInputNode::~MInputNode()
{
}

bool MInputNode::Input(MInputEvent* pEvent, MIViewport* pViewport)
{
	if (m_funcInputCallback)
		return m_funcInputCallback(pEvent, pViewport);

	return false;
}

