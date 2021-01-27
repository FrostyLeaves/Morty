#include "MRenderGraph.h"

#include <queue>
#include <functional>

MRenderGraphNode::MRenderGraphNode(const MString& strNodeName, const std::vector<size_t>& vDependNodes, const std::vector<size_t>& vInputTextures, const std::vector<size_t>& vOutputTextures)
	: m_strNodeName(strNodeName)
	, m_vDependNodeNames(vDependNodes)
	, m_vInputTextureNames(vInputTextures)
	, m_vOutputTextureNames(vOutputTextures)
	, m_nCommandLevel(M_INVALID_INDEX)
{

}

MRenderGraphNode::MRenderGraphNode()
	: m_strNodeName("")
	, m_vDependNodeNames()
	, m_vInputTextureNames()
	, m_vOutputTextureNames()
	, m_nCommandLevel(M_INVALID_INDEX)
{

}

MRenderGraph::MRenderGraph()
	: m_vGraphNodes()
	, m_vGraphTextures()
	, m_nFinalNodeIdx(M_INVALID_INDEX)
{

}

MRenderGraph::~MRenderGraph()
{

}

void MRenderGraph::SetFinalNode(const size_t& nNodeIdx)
{
	m_nFinalNodeIdx = nNodeIdx;
}

size_t MRenderGraph::AddRenderGraphNode(MRenderGraphNode* pNode)
{
	m_vGraphNodes.push_back(pNode);
	return m_vGraphNodes.size() - 1;
}

size_t MRenderGraph::AddRenderGraphTexture(const MString& strTextureName)
{
	//m_vGraphTextures.push_back(strTextureName);
	return m_vGraphTextures.size() - 1;
}

bool MRenderGraph::Compile()
{
	MRenderGraphNode* pFinalNode = GetFinalNode();
	if (!pFinalNode)
		return false;


	pFinalNode->m_nCommandLevel = 0;

	std::queue<MRenderGraphNode*> queue;
	queue.push(pFinalNode);


	MRenderGraphNode* pNode = nullptr;
	while (!queue.empty())
	{
		 pNode = queue.front();
		 queue.pop();

		 for (size_t i = 0; i < pNode->m_vDependNodeNames.size(); ++i)
		 {
			 if (MRenderGraphNode* pChild = GetNode(pNode->m_vDependNodeNames[i]))
			 {
				 if (pChild->m_nCommandLevel < pNode->m_nCommandLevel + 1)
				 {
					 pChild->m_nCommandLevel = pNode->m_nCommandLevel + 1;
				 }

				 queue.push(pChild);
			 }
		 }
	}

	m_vSortedNodes = m_vGraphNodes;

	std::sort(m_vSortedNodes.begin(), m_vSortedNodes.end(), [](MRenderGraphNode* a, MRenderGraphNode* b) {
		return a->m_nCommandLevel > b->m_nCommandLevel;
		});


	return true;
}

MRenderGraphNode* MRenderGraph::GetFinalNode()
{
	return GetNode(m_nFinalNodeIdx);
}

MRenderGraphNode* MRenderGraph::GetNode(const size_t& nIdx)
{
	if (nIdx < m_vGraphNodes.size())
		return m_vGraphNodes[nIdx];

	return nullptr;
}
