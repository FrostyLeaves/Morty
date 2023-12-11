#include "TaskGraph/MTaskNodeInput.h"
#include "TaskGraph/MTaskNodeOutput.h"

#include "TaskGraph/MTaskNode.h"

MTaskNodeInput::MTaskNodeInput()
	: m_unIndex(0)
	, m_strName("")
	, pGraphNode(nullptr)
	, pLinkedOutput(nullptr)
{

}

void MTaskNodeInput::LinkTo(MTaskNodeOutput* pOutput)
{
	UnLink();
	pOutput->LinkTo(this);
}

void MTaskNodeInput::UnLink()
{
	if (pLinkedOutput)
	{
		pLinkedOutput->UnLink(this);
	}
}

MString MTaskNodeInput::GetStringID() const
{
	if (!GetTaskNode())
		return "";

	return GetTaskNode()->GetNodeName().ToString() + "_Input_" + MStringUtil::ToString(m_unIndex);
}

MTaskNode* MTaskNodeInput::GetLinkedNode() const
{
	if (pLinkedOutput)
	{
		return pLinkedOutput->GetTaskNode();
	}

	return nullptr;
}
