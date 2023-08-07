#include "TaskGraph/MTaskGraph.h"

#include "MTaskGraphWalker.h"
#include "Scene/MGuid.h"

MORTY_CLASS_IMPLEMENT(MTaskGraph, MTypeClass)

MTaskGraph::MTaskGraph()
	: MTypeClass()
	, m_bRequireCompile(true)
{

}

MTaskGraph::MTaskGraph(MThreadPool* pThreadPool)
{
	m_pThreadPool = pThreadPool;
}

MTaskGraph::~MTaskGraph()
{
	for (MTaskNode* pTaskNode : m_tTaskNode)
	{
		pTaskNode->OnDelete();
		delete pTaskNode;
		pTaskNode = nullptr;
	}

	m_vStartTaskNode.clear();
	m_vFinalTaskNode.clear();
	m_tTaskNode.clear();
}

bool MTaskGraph::AddNode(const MString& strNodeName, MTaskNode* pNode)
{
	MORTY_ASSERT(!m_bLock);

	if (!pNode)
	{
		return false;
	}
	pNode->m_strNodeName = strNodeName;
	pNode->m_pGraph = this;



	m_tTaskNode.insert(pNode);

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

	MORTY_ASSERT(!m_bLock);
	pTaskNode->DisconnectAll();

	m_tTaskNode.erase(pTaskNode);
	RequireCompile();

	delete pTaskNode;
}

bool MTaskGraph::Compile()
{
	const size_t nSize = m_tTaskNode.size();

	std::queue<MTaskNode*> queue;
	m_vStartTaskNode.clear();
	m_vFinalTaskNode.clear();

	for (auto& pNode : m_tTaskNode)
	{
		pNode->m_nPriorityLevel = 0;

		if (pNode->IsStartNode())
		{
			m_vStartTaskNode.push_back(pNode);
		}

		if (pNode->IsFinalNode())
		{
			m_vFinalTaskNode.push_back(pNode);
			queue.push(pNode);
		}

		pNode->OnCompile();
	}

	if (queue.empty())
	{
		return false;
	}

	MTaskNode* pNode = nullptr;
	while (!queue.empty())
	{
		pNode = queue.front();
		queue.pop();

		for (size_t i = 0; i < pNode->m_vInput.size(); ++i)
		{
			if (MTaskNodeInput* pInput = pNode->m_vInput[i])
			{
				if (MTaskNode* pLinkedNode = pInput->GetLinkedNode())
				{
					if (pLinkedNode->m_nPriorityLevel < pNode->m_nPriorityLevel + 1)
					{
						pLinkedNode->m_nPriorityLevel = pNode->m_nPriorityLevel + 1;
					}

					queue.push(pLinkedNode);
				}

			}
		}
	}

	m_bRequireCompile = false;

	return true;
}

void MTaskGraph::Run()
{
	m_bLock = true;
	MTaskGraphWalker()(this);
	m_bLock = false;
}
