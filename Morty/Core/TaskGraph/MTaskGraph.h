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

class MTaskNode;
class MORTY_API MTaskGraph : MTypeClass
{
    MORTY_CLASS(MTaskGraph)

public:
    MTaskGraph();
	MTaskGraph(MThreadPool* pThreadPool);
    virtual ~MTaskGraph();

    template<typename TYPE>
    TYPE* AddNode(const MString& strNodeName);

	void DestroyNode(MTaskNode* pTaskNode);

    bool Compile();

	void RequireCompile() { m_bRequireCompile = true; }
	bool NeedCompile() const { return m_bRequireCompile; }

	void Run();

	const std::vector<MTaskNode*>& GetStartNodes() { return m_vStartTaskNode; }
	const std::vector<MTaskNode*>& GetFinalNodes() { return m_vFinalTaskNode; }

	MThreadPool* GetThreadPool() const { return m_pThreadPool; }

protected:

	bool AddNode(const MString& strNodeName, MTaskNode* pGraphNode);

	MThreadPool* m_pThreadPool = nullptr;
	std::set<MTaskNode*> m_tTaskNode;

	std::vector<MTaskNode*> m_vStartTaskNode;
	std::vector<MTaskNode*> m_vFinalTaskNode;

	bool m_bRequireCompile;
	bool m_bLock = false;
};

template<typename TYPE>
TYPE* MTaskGraph::AddNode(const MString& strNodeName)
{
	if (!MTypeClass::IsType<TYPE, MTaskNode>())
	{
		return nullptr;
	}

	TYPE* pNode = new TYPE();
	if (!AddNode(strNodeName, pNode))
	{
		delete pNode;
		pNode = nullptr;
	}

	return pNode;
}
