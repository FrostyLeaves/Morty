#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskGraph.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MTimer.h"
#include "Flatbuffer/MTaskNode_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTaskNode, MTypeClass)

const size_t MTaskNode::InvalidSlotId = ~0U;

MTaskNode::~MTaskNode()
{
    for (MTaskNodeInput* input: m_input) { delete input; }
    m_input.clear();

    for (MTaskNodeOutput* output: m_output) { delete output; }
    m_output.clear();
}

void MTaskNode::AppendInput(MTaskNodeInput* input)
{
    input->pGraphNode = this;
    input->m_unIndex  = m_input.size();
    m_input.push_back(input);
}

void MTaskNode::AppendOutput(MTaskNodeOutput* output)
{
    output->pGraphNode = this;
    output->m_unIndex  = m_output.size();

    m_output.push_back(output);
}

MTaskNodeInput* MTaskNode::GetInput(const size_t& nInputIdx) const
{
    if (nInputIdx < m_input.size()) return m_input[nInputIdx];

    return nullptr;
}

MTaskNodeOutput* MTaskNode::GetOutput(const size_t& nOutputIdx) const
{
    if (nOutputIdx < m_output.size()) return m_output[nOutputIdx];

    return nullptr;
}

void MTaskNode::ConnectTo(MTaskNode* pNextNode)
{
    MTaskNodeOutput* output = GetOutputSize() ? GetOutput(0) : AppendOutput();
    MTaskNodeInput*  input  = pNextNode->AppendInput();

    output->LinkTo(input);
}

void MTaskNode::DisconnectTo(MTaskNode* pNextNode)
{
    for (auto iter = m_output.begin(); iter != m_output.end(); ++iter)
    {
        MTaskNodeOutput* output = *iter;
        auto             inputs = output->GetLinkedInputs();
        for (MTaskNodeInput* input: inputs)
        {
            if (input->GetTaskNode() == pNextNode) { output->UnLink(input); }
        }
    }
}


void MTaskNode::DisconnectAll()
{
    for (MTaskNodeInput* input: m_input)
    {
        if (auto pPrevNode = input->GetLinkedNode()) { pPrevNode->DisconnectTo(this); }

        delete input;
    }
    m_input.clear();

    for (MTaskNodeOutput* output: m_output)
    {
        auto inputs = output->GetLinkedInputs();
        for (MTaskNodeInput* input: inputs) { output->UnLink(input); }

        delete output;
    }
    m_output.clear();
}

bool MTaskNode::IsStartNode()
{
    for (MTaskNodeInput* input: m_input)
    {
        if (input->GetLinkedNode()) return false;
    }

    return true;
}

bool MTaskNode::IsFinalNode()
{
    for (MTaskNodeOutput* output: m_output)
    {
        if (!output->GetLinkedInputs().empty()) return false;
    }

    return true;
}

void MTaskNode::Run()
{
#if MORTY_DEBUG
    auto start = MTimer::GetCurTime();
#endif
    if (m_funcTaskFunction) { m_funcTaskFunction(this); }

#if MORTY_DEBUG
    auto end    = MTimer::GetCurTime();
    m_debugTime = ((m_debugTime * 29) + (end - start)) / 30;
#endif
}

flatbuffers::Offset<void> MTaskNode::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    fbs::MTaskNodeBuilder builder(fbb);
    return builder.Finish().Union();
}

void MTaskNode::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto fbNode = fbs::GetMTaskNode(fbb.GetCurrentBufferPointer());
    Deserialize(fbNode);
}

void MTaskNode::Deserialize(const void* flatbuffer) { MORTY_UNUSED(flatbuffer); }