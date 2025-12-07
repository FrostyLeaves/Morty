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
{}

MString MTaskNodeOutput::GetStringID() const
{
    if (!GetTaskNode()) return "";

    return GetTaskNode()->GetNodeName().ToString() + "_Output_" + MStringUtil::ToString(m_unIndex);
}

bool MTaskNodeOutput::LinkTo(MTaskNodeInput* input)
{
    if (!CanLink(input)) { return false; }

    if (input)
    {
        UNION_PUSH_BACK_VECTOR(vLinkedInput, input);
        input->pLinkedOutput = this;
    }

    return true;
}

void MTaskNodeOutput::UnLink(MTaskNodeInput* input)
{
    if (input->pLinkedOutput == this)
    {
        ERASE_FIRST_VECTOR(vLinkedInput, input);
        input->pLinkedOutput = nullptr;
    }
}

bool MTaskNodeOutput::CanLink(const MTaskNodeInput* input) const
{
    MORTY_UNUSED(input);
    return true;
}
