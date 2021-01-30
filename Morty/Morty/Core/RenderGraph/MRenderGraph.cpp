#include "MRenderGraph.h"

#include "MFunction.h"
#include "MRenderPass.h"


#include <queue>
#include <functional>

M_OBJECT_IMPLEMENT(MRenderGraph, MObject)

MRenderGraphNode::MRenderGraphNode()
	: m_strNodeName("")
	, m_vInputTextures()
	, m_vOutputTextures()
	, m_nCommandLevel(M_INVALID_INDEX)
	, m_bFinalNode(false)
	, m_pGraph(nullptr)
	, m_pRenderPass(new MRenderPass())
{

}

MRenderGraphNodeInput* MRenderGraphNode::AppendInput()
{
	MRenderGraphNodeInput* pInput = new MRenderGraphNodeInput();
	m_vInputTextures.push_back(pInput);

	return pInput;
}

MRenderGraphNodeOutput* MRenderGraphNode::AppendOutput()
{
	MRenderGraphNodeOutput* pOutput = new MRenderGraphNodeOutput();
	pOutput->pGraphNode = this;

	m_vOutputTextures.push_back(pOutput);

	return pOutput;
}

MRenderGraphNodeInput* MRenderGraphNode::GetInput(const size_t& nInputIdx)
{
	if (nInputIdx < m_vInputTextures.size())
		return m_vInputTextures[nInputIdx];

	return nullptr;
}

MRenderGraphNodeOutput* MRenderGraphNode::GetOutput(const size_t& nOutputIdx)
{
	if (nOutputIdx < m_vOutputTextures.size())
		return m_vOutputTextures[nOutputIdx];

	return nullptr;
}

MRenderGraphTexture* MRenderGraphNode::GetInputTexture(const size_t& nInputIdx)
{
	if (MRenderGraphNodeInput* pInput = GetInput(nInputIdx))
	{
		return pInput->GetLinkedTexture();
	}

	return nullptr;
}

void MRenderGraphNode::Compile(MIDevice* pDevice)
{
	DestroyBuffer(pDevice);

	m_pRenderPass->m_aFrameBuffers.clear();
	m_pRenderPass->m_aFrameBuffers.resize(M_BUFFER_NUM);

	for (size_t opIdx = 0; opIdx < m_vOutputTextures.size(); ++opIdx)
	{
		MRenderGraphNodeOutput* pOutput = m_vOutputTextures[opIdx];
		if (MRenderGraphTexture* pRenderTexture = pOutput->GetRenderTexture())
		{

			if (pRenderTexture->GetUsage() == METextureUsage::ERenderBack)
			{
				for (size_t fbIdx = 0; fbIdx < M_BUFFER_NUM; ++fbIdx)
				{
					MFrameBuffer& frameBuffer = m_pRenderPass->m_aFrameBuffers[fbIdx];
					frameBuffer.vBackTextures.push_back(pRenderTexture->GetRenderTexture(fbIdx));
				}

				m_pRenderPass->m_vBackDesc[opIdx].bClearWhenRender = pOutput->GetClear();
				m_pRenderPass->m_vBackDesc[opIdx].cClearColor = pOutput->GetClearColor();
			}
			else if (pRenderTexture->GetUsage() == METextureUsage::ERenderDepth)
			{
				for (size_t fbIdx = 0; fbIdx < M_BUFFER_NUM; ++fbIdx)
				{
					MFrameBuffer& frameBuffer = m_pRenderPass->m_aFrameBuffers[fbIdx];
					frameBuffer.pDepthTexture = pRenderTexture->GetRenderTexture(fbIdx);
				}

				m_pRenderPass->m_DepthDesc.bClearWhenRender = pOutput->GetClear();
			}
			else
			{
				//error
			}

		}
		else
		{
			//error.
		}
	}
}

void MRenderGraphNode::GenerateBuffer(MIDevice* pDevice)
{
	
}

void MRenderGraphNode::DestroyBuffer(MIDevice* pDevice)
{
	m_pRenderPass->DestroyBuffer(pDevice);
}

MRenderGraph::MRenderGraph()
	: m_tGraphNodeMap()
	, m_tGraphTextureMap()
	, m_bCompiled(false)
{

}

MRenderGraph::~MRenderGraph()
{

}

MRenderGraphNode* MRenderGraph::AddRenderGraphNode(const MString& strNodeName)
{
	if (MRenderGraphNode* pFindResult = FindRenderGraphNode(strNodeName))
	{
		return pFindResult;
	}

	MRenderGraphNode* pGraphNode = new MRenderGraphNode();

	pGraphNode->m_strNodeName = strNodeName;
	pGraphNode->m_pGraph = this;

	m_tGraphNodeMap[pGraphNode->m_strNodeName] = pGraphNode;

	return pGraphNode;
}

MRenderGraphTexture* MRenderGraph::AddRenderGraphTexture(const MString& strTextureName)
{
	if (MRenderGraphTexture* pFindResult = FindRenderGraphTexture(strTextureName))
	{
		return pFindResult;
	}

	MRenderGraphTexture* pGraphTexture = new MRenderGraphTexture();
	pGraphTexture->m_strTextureName = strTextureName;

	m_tGraphTextureMap[strTextureName] = pGraphTexture;

	return pGraphTexture;
}

MRenderGraphTexture* MRenderGraph::FindRenderGraphTexture(const MString& strTextureName) const
{
	auto iter = m_tGraphTextureMap.find(strTextureName);
	if (iter != m_tGraphTextureMap.end())
		return iter->second;

	return nullptr;
}

MRenderGraphNode* MRenderGraph::FindRenderGraphNode(const MString& strNodeName) const
{
	auto iter = m_tGraphNodeMap.find(strNodeName);
	if (iter != m_tGraphNodeMap.end())
		return iter->second;

	return nullptr;
}

void MRenderGraph::CompileDirty()
{
	m_bCompiled = false;
}

bool MRenderGraph::Compile(MIDevice* pDevice)
{
	if (m_bCompiled)
		return true;

	const size_t nSize = m_tGraphNodeMap.size();

	m_vSortedNodes.clear();
	for (auto& pr : m_tGraphNodeMap)
	{
		pr.second->m_nCommandLevel = M_INVALID_INDEX;
		m_vSortedNodes.push_back(pr.second);
	}

	std::queue<MRenderGraphNode*> queue;
	for (size_t nodeIdx = 0; nodeIdx < nSize; ++nodeIdx)
	{
		if (m_vSortedNodes[nodeIdx]->GetFinalNode())
		{
			m_vSortedNodes[nodeIdx]->m_nCommandLevel = 0;
			queue.push(m_vSortedNodes[nodeIdx]);
		}
	}

	if (queue.empty())
	{
		return false;
	}

	MRenderGraphNode* pNode = nullptr;
	while (!queue.empty())
	{
		 pNode = queue.front();
		 queue.pop();

		 for (size_t i = 0; i < pNode->m_vInputTextures.size(); ++i)
		 {
			 if (MRenderGraphNodeInput* pInput = pNode->m_vInputTextures[i])
			 {
				 if (MRenderGraphNode* pLinkedNode = pInput->GetLinkedNode())
				 {
					 if (pLinkedNode->m_nCommandLevel < pNode->m_nCommandLevel + 1)
					 {
						 pLinkedNode->m_nCommandLevel = pNode->m_nCommandLevel + 1;
					 }

					 queue.push(pLinkedNode);
				 }

			 }
		 }
	}

	std::sort(m_vSortedNodes.begin(), m_vSortedNodes.end(), [](MRenderGraphNode* a, MRenderGraphNode* b) {
		return a->m_nCommandLevel > b->m_nCommandLevel;
		});

	for (auto& pr : m_tGraphTextureMap)
	{
		pr.second->Compile(pDevice);
	}

	for (MRenderGraphNode* pNode : m_vSortedNodes)
	{
		pNode->Compile(pDevice);
	}

	return m_bCompiled =  true;
}

void MRenderGraph::Render()
{
	if (!m_bCompiled)
		return;

	for (MRenderGraphNode* pNode : m_vSortedNodes)
	{
		if (pNode->m_funcRender)
		{
			pNode->m_funcRender(pNode);
		}
	}
}

void MRenderGraph::GenerateBuffer(MIDevice* pDevice)
{
	if (!m_bCompiled && !Compile(pDevice))
		return;
	
	for (MRenderGraphNode* pGraphNode : m_vSortedNodes)
	{
		pGraphNode->GenerateBuffer(pDevice);
	}
}

void MRenderGraph::DestroyBuffer(MIDevice* pDevice)
{
	for (MRenderGraphNode* pGraphNode : m_vSortedNodes)
	{
		pGraphNode->DestroyBuffer(pDevice);
	}
}

MRenderGraphTexture::MRenderGraphTexture()
	: m_strTextureName()
	, m_aTextures()
	, m_eUsage(METextureUsage::ERenderBack)
	, m_eLayout(METextureLayout::ERGBA8)
	, m_v2Size()
	, m_pGraph(nullptr)
{

}

void MRenderGraphTexture::SetUsage(const METextureUsage& eUsage)
{
	m_eUsage = eUsage;
}

void MRenderGraphTexture::SetLayout(const METextureLayout& eLayout)
{
	m_eLayout = eLayout;
}

MIRenderTexture* MRenderGraphTexture::GetRenderTexture(const size_t& nIdx)
{
	if (nIdx < m_aTextures.size())
		return m_aTextures[nIdx];

	return nullptr;
}

void MRenderGraphTexture::Compile(MIDevice* pDevice)
{
	for (size_t texIdx = 0; texIdx < m_aTextures.size(); ++texIdx)
	{
		m_aTextures[texIdx]->DestroyBuffer(pDevice);

		m_aTextures[texIdx]->SetUsage(m_eUsage);
		m_aTextures[texIdx]->SetType(m_eLayout);
		m_aTextures[texIdx]->SetSize(m_v2Size);
	}
}

MRenderGraphNodeOutput::MRenderGraphNodeOutput()
	: bClear(true)
	, mClearColor(MColor::Black_T)
	, pGraphNode(nullptr)
	, pGraphTexture(nullptr)
	, vLinkedInput()
{

}

void MRenderGraphNodeOutput::LinkTo(MRenderGraphNodeInput* pInput)
{
	if (pInput && UNION_PUSH_BACK_VECTOR(vLinkedInput, pInput))
	{
		pInput->pLinkedOutput = this;
	}
}

void MRenderGraphNodeOutput::UnLink(MRenderGraphNodeInput* pInput)
{
	if (pInput->pLinkedOutput == this)
	{
		ERASE_FIRST_VECTOR(vLinkedInput, pInput);
		pInput->pLinkedOutput = nullptr;
	}
}

void MRenderGraphNodeInput::LinkTo(MRenderGraphNodeOutput* pOutput)
{
	UnLink();
	pOutput->LinkTo(this);
}

void MRenderGraphNodeInput::UnLink()
{
	if (pLinkedOutput)
	{
		pLinkedOutput->UnLink(this);
	}
}

MRenderGraphNode* MRenderGraphNodeInput::GetLinkedNode() const
{
	if (pLinkedOutput)
	{
		return pLinkedOutput->GetRenderGraphNode();
	}

	return nullptr;
}

MRenderGraphTexture* MRenderGraphNodeInput::GetLinkedTexture() const
{
	if (pLinkedOutput)
	{
		return pLinkedOutput->GetRenderTexture();
	}

	return nullptr;
}
