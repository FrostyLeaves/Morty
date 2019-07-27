#include "M3DNode.h"

M3DNode::M3DNode()
	: MNode()
{

}

M3DNode::~M3DNode()
{

}

M3DNode* M3DNode::Create()
{
	M3DNode* pNode = new M3DNode();

	return pNode;
}
