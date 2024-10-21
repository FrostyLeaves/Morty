#include "TaskGraph/MTaskGraph.h"

#include "MTaskGraphWalker.h"
#include "Scene/MGuid.h"
#include "Flatbuffer/MTaskGraph_generated.h"
#include "Flatbuffer/MTaskNode_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTaskGraph, MTypeClass)

MTaskGraph::MTaskGraph()
    : MTypeClass()
    , m_requireCompile(true)
{}

MTaskGraph::~MTaskGraph()
{
    for (auto pair: m_taskNode)
    {
        pair.second->OnDelete();
        MORTY_SAFE_DELETE(pair.second);
    }

    m_startTaskNode.clear();
    m_finalTaskNode.clear();
    m_taskNode.clear();
}

bool MTaskGraph::AddNode(const MStringId& strNodeName, MTaskNode* pTaskNode)
{
    MORTY_ASSERT(!m_lock);

    if (!pTaskNode) { return false; }
    pTaskNode->m_strNodeName    = strNodeName;
    pTaskNode->m_graph          = this;
    pTaskNode->m_id             = m_idPool.GetNewID();
    m_taskNode[pTaskNode->m_id] = pTaskNode;

    pTaskNode->OnCreated();
    RequireCompile();
    return true;
}

void MTaskGraph::DestroyNode(MTaskNode* pTaskNode)
{
    if (!pTaskNode)
    {
        MORTY_ASSERT(pTaskNode);
        return;
    }

    MORTY_ASSERT(!m_lock);
    pTaskNode->DisconnectAll();

    m_taskNode.erase(pTaskNode->GetNodeID());
    m_idPool.RecoveryID(pTaskNode->GetNodeID());

    RequireCompile();

    delete pTaskNode;
}

bool MTaskGraph::Compile()
{
    const size_t           nTaskNodeNum = m_taskNode.size();

    std::queue<MTaskNode*> queue;
    m_startTaskNode.clear();
    m_finalTaskNode.clear();

    for (const auto& pr: m_taskNode)
    {
        auto pNode             = pr.second;
        pNode->m_priorityLevel = 0;

        if (pNode->IsStartNode()) { m_startTaskNode.push_back(pNode); }

        if (pNode->IsFinalNode())
        {
            m_finalTaskNode.push_back(pNode);
            queue.push(pNode);
        }

        pNode->OnCompile();
    }

    if (queue.empty()) { return false; }

    MTaskNode* pNode = nullptr;
    while (!queue.empty())
    {
        pNode = queue.front();
        queue.pop();

        for (size_t i = 0; i < pNode->m_input.size(); ++i)
        {
            if (MTaskNodeInput* pInput = pNode->m_input[i])
            {
                if (MTaskNode* pLinkedNode = pInput->GetLinkedNode())
                {
                    if (pLinkedNode->m_priorityLevel < pNode->m_priorityLevel + 1)
                    {
                        pLinkedNode->m_priorityLevel = pNode->m_priorityLevel + 1;
                        MORTY_ASSERT(pLinkedNode->m_priorityLevel < nTaskNodeNum);
                    }

                    queue.push(pLinkedNode);
                }
            }
        }
    }

    m_requireCompile = false;
    OnPostCompile();

    return true;
}

void MTaskGraph::Run(ITaskGraphWalker* pWalker)
{
    m_lock = true;
    (*pWalker)(this);
    m_lock = false;
}

std::vector<MTaskNode*> MTaskGraph::GetOrderedNodes()
{
    std::vector<MTaskNode*> vNodes(m_taskNode.size());
    std::transform(m_taskNode.begin(), m_taskNode.end(), vNodes.begin(), [](auto node) { return node.second; });
    std::sort(vNodes.begin(), vNodes.end(), [](MTaskNode* a, MTaskNode* b) {
        return a->m_priorityLevel > b->m_priorityLevel;
    });

    return vNodes;
}

std::vector<MTaskNode*> MTaskGraph::GetAllNodes()
{
    std::vector<MTaskNode*> vNodes(m_taskNode.size());
    std::transform(m_taskNode.begin(), m_taskNode.end(), vNodes.begin(), [](auto node) { return node.second; });

    return vNodes;
}
MTaskNode* MTaskGraph::FindTaskNode(size_t id) const
{
    auto findResult = m_taskNode.find(id);
    if (findResult == m_taskNode.end()) { return nullptr; }
    return findResult->second;
}

flatbuffers::Offset<void> MTaskGraph::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    std::vector<flatbuffers::Offset<fbs::MTaskNodeDesc>> nodes;
    for (auto& taskNode: GetAllNodes())
    {
        std::vector<uint32_t> taskInputNode(taskNode->GetInputSize());
        std::vector<uint32_t> taskInputSlot(taskNode->GetInputSize());

        for (size_t nInputIdx = 0; nInputIdx < taskNode->GetInputSize(); ++nInputIdx)
        {
            auto prevOutput          = taskNode->GetInput(nInputIdx)->GetLinkedOutput();
            taskInputNode[nInputIdx] = prevOutput ? prevOutput->GetTaskNode()->GetNodeID() : MTaskNode::InvalidSlotId;
            taskInputSlot[nInputIdx] = prevOutput ? prevOutput->GetIndex() : MTaskNode::InvalidSlotId;
        }
        auto                           fbTypeName  = fbb.CreateString(taskNode->GetTypeName());
        auto                           fbInputNode = fbb.CreateVector(taskInputNode);
        auto                           fbInputSlot = fbb.CreateVector(taskInputSlot);


        flatbuffers::FlatBufferBuilder nodeDataFbb;
        nodeDataFbb.Finish(taskNode->Serialize(nodeDataFbb));

        std::vector<MByte> nodeData(nodeDataFbb.GetSize());
        memcpy(nodeData.data(), nodeDataFbb.GetBufferPointer(), nodeDataFbb.GetSize() * sizeof(MByte));
        auto                      fbNodeData = fbb.CreateVector(nodeData);

        fbs::MTaskNodeDescBuilder nodeBuilder(fbb);
        nodeBuilder.add_node_id(taskNode->GetNodeID());
        nodeBuilder.add_node_type(fbTypeName);
        nodeBuilder.add_link_node_id(fbInputNode.o);
        nodeBuilder.add_link_output_id(fbInputSlot.o);
        nodeBuilder.add_data(fbNodeData.o);

        return nodeBuilder.Finish().Union();
    }
    auto                   fbNodes = fbb.CreateVector(nodes);

    fbs::MTaskGraphBuilder builder(fbb);
    builder.add_node_array(fbNodes);
    return builder.Finish().Union();
}
void MTaskGraph::Deserialize(const void* pBufferPointer)
{
    const fbs::MTaskGraph*                   fbTaskGraph = reinterpret_cast<const fbs::MTaskGraph*>(pBufferPointer);

    const auto&                              fbTaskNodeArray = *fbTaskGraph->node_array();

    std::unordered_map<uint32_t, MTaskNode*> taskNodes;

    for (size_t nodeIdx = 0; nodeIdx < fbTaskNodeArray.size(); ++nodeIdx)
    {
        const fbs::MTaskNodeDesc* fbTaskNodeDesc = fbTaskNodeArray.Get(static_cast<flatbuffers::uoffset_t>(nodeIdx));
        auto                      nodeTypeName   = fbTaskNodeDesc->node_type()->str();
        auto                      nodeId         = fbTaskNodeDesc->node_id();
        auto                      taskNode       = MTypeClass::New(nodeTypeName)->DynamicCast<MTaskNode>();
        auto                      fbTaskNodeData = fbTaskNodeDesc->data();

        flatbuffers::FlatBufferBuilder nodeFbb;
        nodeFbb.PushBytes((const uint8_t*) fbTaskNodeData->data(), fbTaskNodeData->size());
        taskNode->Deserialize(nodeFbb.GetCurrentBufferPointer());
        taskNodes[nodeId] = taskNode;

        MORTY_ASSERT(AddNode(taskNode->GetNodeName(), taskNode));
    }

    for (size_t idx = 0; idx < fbTaskNodeArray.size(); ++idx)
    {
        const fbs::MTaskNodeDesc* fbTaskNodeDesc = fbTaskNodeArray.Get(static_cast<flatbuffers::uoffset_t>(idx));
        if (fbTaskNodeDesc->link_node_id() == nullptr || fbTaskNodeDesc->link_output_id() == nullptr) continue;
        const auto   nodeIdx = fbTaskNodeDesc->node_id();

        MTaskNode*   taskNode   = taskNodes[nodeIdx];
        const size_t connectNum = fbTaskNodeDesc->link_node_id()->size();

        for (size_t connIdx = 0; connIdx < connectNum; ++connIdx)
        {
            const auto prevNodeIdx   = fbTaskNodeDesc->link_node_id()->Get(connIdx);
            const auto prevOutputIdx = fbTaskNodeDesc->link_output_id()->Get(connIdx);

            if (prevNodeIdx != MTaskNode::InvalidSlotId && prevOutputIdx != MTaskNode::InvalidSlotId)
            {
                auto prevNode = taskNodes[prevNodeIdx];
                prevNode->GetOutput(prevOutputIdx)->LinkTo(taskNode->GetInput(connIdx));
            }
        }
    }
}
