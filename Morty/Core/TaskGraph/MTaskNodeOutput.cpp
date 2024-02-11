#include "TaskGraph/MTaskNodeOutput.h"
#include "TaskGraph/MTaskNodeInput.h"

#include "TaskGraph/MTaskNode.h"
#include "Utility/MFunction.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTaskNodeOutput, MTypeClass)

MTaskNodeOutput::MTaskNodeOutput()
	: m_unIndex(0)
	, m_strName("")
	, pGraphNode(nullptr)
	, vLinkedInput()
{

}

MString MTaskNodeOutput::GetStringID() const
{
	if (!GetTaskNode())
		return "";

	return GetTaskNode()->GetNodeName().ToString() + "_Output_" + MStringUtil::ToString(m_unIndex);
}

void MTaskNodeOutput::LinkTo(MTaskNodeInput* pInput)
{
	if (pInput && UNION_PUSH_BACK_VECTOR(vLinkedInput, pInput))
	{
		pInput->pLinkedOutput = this;
	}
}

void MTaskNodeOutput::UnLink(MTaskNodeInput* pInput)
{
	if (pInput->pLinkedOutput == this)
	{
		ERASE_FIRST_VECTOR(vLinkedInput, pInput);
		pInput->pLinkedOutput = nullptr;
	}
}
