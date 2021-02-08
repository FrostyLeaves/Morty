#include "MRenderGraph.h"

#include "MEngine.h"
#include "MFunction.h"
#include "MRenderPass.h"


#include <queue>
#include <functional>


MRenderGraphNode::MRenderGraphNode()
	: m_strNodeName("")
	, m_vInputTextures()
	, m_vOutputTextures()
	, m_nCommandLevel(M_INVALID_INDEX)
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

	for (size_t opIdx = 0; opIdx < m_vOutputTextures.size(); ++opIdx)
	{
		MRenderGraphNodeOutput* pOutput = m_vOutputTextures[opIdx];
		if (MRenderGraphTexture* pRenderTexture = pOutput->GetRenderTexture())
		{

			if (pRenderTexture->GetUsage() == METextureUsage::ERenderBack)
			{
				MFrameBuffer* pFrameBuffer = m_pRenderPass->GetFrameBuffer();
				pFrameBuffer->vBackTextures.push_back(pRenderTexture->GetRenderTexture());

				m_pRenderPass->m_vBackDesc.push_back({});
				m_pRenderPass->m_vBackDesc.back().bClearWhenRender = pOutput->GetClear();
				m_pRenderPass->m_vBackDesc.back().cClearColor = pOutput->GetClearColor();
			}
			else if (pRenderTexture->GetUsage() == METextureUsage::ERenderDepth)
			{
				MFrameBuffer* pFrameBuffer = m_pRenderPass->GetFrameBuffer();
				pFrameBuffer->pDepthTexture = pRenderTexture->GetRenderTexture();

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
	, m_pEngine(nullptr)
	, m_pFinalOutputTexture(nullptr)
{

}

MRenderGraph::MRenderGraph(MEngine* pEngine)
	: m_tGraphNodeMap()
	, m_tGraphTextureMap()
	, m_bCompiled(false)
	, m_pEngine(pEngine)
	, m_pFinalOutputTexture(nullptr)
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

	MRenderGraphNode* pGraphNode = NewRenderGraphNode();

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
	pGraphTexture->m_pGraph = this;

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

void MRenderGraph::SetFinalOutputTexture(MRenderGraphTexture* pGraphTexture)
{
	if (!pGraphTexture || pGraphTexture->GetRenderGraph() == this)
	{
		m_pFinalOutputTexture = pGraphTexture;
	}
}

void MRenderGraph::SetFinalNode(MRenderGraphNode* pGraphNode)
{
	if (!pGraphNode || pGraphNode->m_pGraph == this)
	{
		m_pFinalNode = pGraphNode;
	}
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
	queue.push(GetFinalNode());

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

void MRenderGraph::GenerateBuffer(MIDevice* pDevice)
{
	if (!m_bCompiled && !Compile(pDevice))
		return;
	
	for (auto& pr : m_tGraphTextureMap)
	{
		pr.second->GenerateBuffer(pDevice);
	}

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

	for (auto& pr : m_tGraphTextureMap)
	{
		pr.second->DestroyBuffer(pDevice);
		pr.second = nullptr;
	}
}

MRenderGraphTexture::MRenderGraphTexture()
	: m_strTextureName()
	, m_pTexture(nullptr)
	, m_eUsage(METextureUsage::ERenderBack)
	, m_eLayout(METextureLayout::ERGBA8)
	, m_v2Size()
	, m_pGraph(nullptr)
{
	m_pTexture = new MRenderTexture();
}

void MRenderGraphTexture::SetUsage(const METextureUsage& eUsage)
{
	m_eUsage = eUsage;
}

void MRenderGraphTexture::SetLayout(const METextureLayout& eLayout)
{
	m_eLayout = eLayout;
}

void MRenderGraphTexture::SetSize(const Vector2& size)
{
	if (m_v2Size != size)
	{
		m_pTexture->DestroyBuffer(GetRenderGraph()->GetEngine()->GetDevice());

		m_pTexture->SetUsage(m_eUsage);
		m_pTexture->SetType(m_eLayout);
		m_pTexture->SetSize(m_v2Size);

//		m_aTextures[texIdx]->GenerateBuffer(pDevice);


		for (MRenderGraphNodeOutput* pOutput : m_vOutputs)
		{
			if (MRenderGraphNode* pOwnerNode = pOutput->GetRenderGraphNode())
			{
				pOwnerNode->DestroyBuffer(GetRenderGraph()->GetEngine()->GetDevice());
			}
		}

		m_v2Size = size;
	}
}

void MRenderGraphTexture::AddRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput)
{
	UNION_PUSH_BACK_VECTOR(m_vOutputs, pOutput);
}

void MRenderGraphTexture::RemoveRenderGraphNodeOutput(MRenderGraphNodeOutput* pOutput)
{
	ERASE_FIRST_VECTOR(m_vOutputs, pOutput);
}

MIRenderTexture* MRenderGraphTexture::GetRenderTexture()
{
	return m_pTexture;
}

void MRenderGraphTexture::Compile(MIDevice* pDevice)
{
	m_pTexture->DestroyBuffer(pDevice);

	m_pTexture->SetUsage(m_eUsage);
	m_pTexture->SetType(m_eLayout);
	m_pTexture->SetSize(m_v2Size);

	m_pTexture->GenerateBuffer(pDevice);
}

void MRenderGraphTexture::GenerateBuffer(MIDevice* pDevice)
{
	m_pTexture->GenerateBuffer(pDevice);
}

void MRenderGraphTexture::DestroyBuffer(MIDevice* pDevice)
{
	m_pTexture->DestroyBuffer(pDevice);
}

MRenderGraphNodeOutput::MRenderGraphNodeOutput()
	: bClear(true)
	, mClearColor(MColor::Black_T)
	, pGraphNode(nullptr)
	, pGraphTexture(nullptr)
	, vLinkedInput()
{

}

void MRenderGraphNodeOutput::SetRenderTexture(MRenderGraphTexture* pTexture)
{
	if (pGraphTexture)
	{
		pGraphTexture->RemoveRenderGraphNodeOutput(this);
	}

	if (pGraphTexture = pTexture)
	{
		pGraphTexture->AddRenderGraphNodeOutput(this);
	}
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
