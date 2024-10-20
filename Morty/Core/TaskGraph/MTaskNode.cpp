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
    for (MTaskNodeInput* pInput: m_input) { delete pInput; }
    m_input.clear();

    for (MTaskNodeOutput* pOutput: m_output) { delete pOutput; }
    m_output.clear();
}

void MTaskNode::AppendInput(MTaskNodeInput* pInput)
{
    pInput->pGraphNode = this;
    pInput->m_unIndex  = m_input.size();
    m_input.push_back(pInput);
}

void MTaskNode::AppendOutput(MTaskNodeOutput* pOutput)
{
    pOutput->pGraphNode = this;
    pOutput->m_unIndex  = m_output.size();

    m_output.push_back(pOutput);
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
    MTaskNodeOutput* pOutput = GetOutputSize() ? GetOutput(0) : AppendOutput();
    MTaskNodeInput*  pInput  = pNextNode->AppendInput();

    pOutput->LinkTo(pInput);
}

void MTaskNode::DisconnectTo(MTaskNode* pNextNode)
{
    for (auto iter = m_output.begin(); iter != m_output.end(); ++iter)
    {
        MTaskNodeOutput* pOutput = *iter;
        auto             inputs  = pOutput->GetLinkedInputs();
        for (MTaskNodeInput* pInput: inputs)
        {
            if (pInput->GetTaskNode() == pNextNode) { pOutput->UnLink(pInput); }
        }
    }
}


void MTaskNode::DisconnectAll()
{
    for (MTaskNodeInput* pInput: m_input)
    {
        if (auto pPrevNode = pInput->GetLinkedNode()) { pPrevNode->DisconnectTo(this); }

        delete pInput;
    }
    m_input.clear();

    for (MTaskNodeOutput* pOutput: m_output)
    {
        auto inputs = pOutput->GetLinkedInputs();
        for (MTaskNodeInput* pInput: inputs) { pOutput->UnLink(pInput); }

        delete pOutput;
    }
    m_output.clear();
}

bool MTaskNode::IsStartNode()
{
    for (MTaskNodeInput* pInput: m_input)
    {
        if (pInput->GetLinkedNode()) return false;
    }

    return true;
}

bool MTaskNode::IsFinalNode()
{
    for (MTaskNodeOutput* pOutput: m_output)
    {
        if (!pOutput->GetLinkedInputs().empty()) return false;
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
    auto                    extent = Serialize(fbb);
    auto                    type   = fbb.CreateString(GetTypeName());

    fbs::AnyTaskNodeBuilder builder(fbb);
    builder.add_data(extent.o);
    builder.add_type(type);

    auto                  valus = builder.Finish().Union();

    std::vector<uint32_t> taskInputNode(GetInputSize());
    std::vector<uint32_t> taskInputSlot(GetInputSize());

    for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
    {
        auto prevOutput          = GetInput(nInputIdx)->GetLinkedOutput();
        taskInputNode[nInputIdx] = prevOutput ? prevOutput->GetTaskNode()->GetNodeID() : MTaskNode::InvalidSlotId;
        taskInputSlot[nInputIdx] = prevOutput ? prevOutput->GetIndex() : MTaskNode::InvalidSlotId;
    }
    auto                  fbTypeName  = fbb.CreateString(GetTypeName());
    auto                  fbInputNode = fbb.CreateVector(taskInputNode);
    auto                  fbInputSlot = fbb.CreateVector(taskInputSlot);

    fbs::MTaskNodeBuilder taskNodeBuilder(fbb);
    taskNodeBuilder.add_node_id(GetNodeID());
    taskNodeBuilder.add_node_type(fbTypeName);
    taskNodeBuilder.add_link_node_id(fbInputNode.o);
    taskNodeBuilder.add_link_output_id(fbInputSlot.o);
    taskNodeBuilder.add_node(valus.o);

    return taskNodeBuilder.Finish().Union();
}
