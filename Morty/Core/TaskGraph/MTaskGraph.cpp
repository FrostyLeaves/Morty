#include "TaskGraph/MTaskGraph.h"

#include "MTaskGraphWalker.h"

MORTY_CLASS_IMPLEMENT(MTaskGraph, MObject)

MTaskGraph::MTaskGraph()
	: MObject()
	, m_bRequireCompile(true)
	, m_bValid(true)
{

}

MTaskGraph::~MTaskGraph()
{

}

bool MTaskGraph::AddNode(const MString& strNodeName, MTaskNode* pNode)
{
	if (!pNode)
		return false;

	pNode->m_strNodeName = strNodeName;
	pNode->m_pGraph = this;

	m_tTaskNode.insert(pNode);
	m_vTaskNode.push_back(pNode);

	pNode->OnCreated();
	return true;
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
	m_bValid = true;

	return true;
}

void MTaskGraph::Run()
{
	MTaskGraphWalker()(this);
}

void MTaskGraph::OnDelete()
{
	for (MTaskNode* pTaskNode : m_vTaskNode)
	{
		pTaskNode->OnDelete();
		delete pTaskNode;
		pTaskNode = nullptr;
	}

	m_vStartTaskNode.clear();
	m_vFinalTaskNode.clear();
	m_tTaskNode.clear();

	m_bValid = false;
}
