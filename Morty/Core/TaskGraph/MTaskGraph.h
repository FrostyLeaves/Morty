/**
 * @File         MTaskGraph
 * 
 * @Created      2021-07-08 11:26:02
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Thread/MThreadPool.h"

namespace morty
{

class MTaskNode;
class ITaskGraphWalker;
class MORTY_API MTaskGraph : public MTypeClass
{
    MORTY_CLASS(MTaskGraph)

public:
    MTaskGraph();
    ~MTaskGraph() override;

    template<typename TYPE> TYPE*                AddNode(const MStringId& strNodeName);

    void                                         DestroyNode(MTaskNode* pTaskNode);

    bool                                         Compile();

    void                                         Run(ITaskGraphWalker* pWalker);

    void                                         RequireCompile() { m_requireCompile = true; }
    [[nodiscard]] bool                           NeedCompile() const { return m_requireCompile; }

    [[nodiscard]] const std::vector<MTaskNode*>& GetStartNodes() const { return m_startTaskNode; }
    [[nodiscard]] const std::vector<MTaskNode*>& GetFinalNodes() const { return m_finalTaskNode; }
    std::vector<MTaskNode*>                      GetOrderedNodes();
    std::vector<MTaskNode*>                      GetAllNodes();

protected:
    bool                    AddNode(const MStringId& strNodeName, MTaskNode* pGraphNode);

    std::set<MTaskNode*>    m_taskNode;

    std::vector<MTaskNode*> m_startTaskNode;
    std::vector<MTaskNode*> m_finalTaskNode;

    bool                    m_requireCompile;
    bool                    m_lock = false;
};

template<typename TYPE> TYPE* MTaskGraph::AddNode(const MStringId& strNodeName)
{
    if (!MTypeClass::IsType<TYPE, MTaskNode>()) { return nullptr; }

    TYPE* pNode = new TYPE();
    if (!AddNode(strNodeName, pNode))
    {
        delete pNode;
        pNode = nullptr;
    }

    return pNode;
}

}// namespace morty