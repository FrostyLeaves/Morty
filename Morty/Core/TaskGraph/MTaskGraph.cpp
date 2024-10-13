#include "TaskGraph/MTaskGraph.h"

#include "MTaskGraphWalker.h"
#include "Scene/MGuid.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTaskGraph, MTypeClass)

MTaskGraph::MTaskGraph()
    : MTypeClass()
    , m_requireCompile(true)
{}

MTaskGraph::~MTaskGraph()
{
    for (MTaskNode* pTaskNode: m_taskNode)
    {
        pTaskNode->OnDelete();
        delete pTaskNode;
        pTaskNode = nullptr;
    }

    m_startTaskNode.clear();
    m_finalTaskNode.clear();
    m_taskNode.clear();
}

bool MTaskGraph::AddNode(const MStringId& strNodeName, MTaskNode* pNode)
{
    MORTY_ASSERT(!m_lock);

    if (!pNode) { return false; }
    pNode->m_strNodeName = strNodeName;
    pNode->m_graph       = this;


    m_taskNode.insert(pNode);

    pNode->OnCreated();
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

    m_taskNode.erase(pTaskNode);

    RequireCompile();

    delete pTaskNode;
}

bool MTaskGraph::Compile()
{
    const size_t           nTaskNodeNum = m_taskNode.size();

    std::queue<MTaskNode*> queue;
    m_startTaskNode.clear();
    m_finalTaskNode.clear();

    for (auto& pNode: m_taskNode)
    {
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
    std::transform(m_taskNode.begin(), m_taskNode.end(), vNodes.begin(), [](auto node) { return node; });
    std::sort(vNodes.begin(), vNodes.end(), [](MTaskNode* a, MTaskNode* b) {
        return a->m_priorityLevel > b->m_priorityLevel;
    });

    return vNodes;
}

std::vector<MTaskNode*> MTaskGraph::GetAllNodes()
{
    std::vector<MTaskNode*> vNodes(m_taskNode.size());
    std::transform(m_taskNode.begin(), m_taskNode.end(), vNodes.begin(), [](auto node) { return node; });

    return vNodes;
}
