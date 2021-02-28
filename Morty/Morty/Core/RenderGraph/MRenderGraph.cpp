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
	, m_bDirty(true)
	, m_funcRender(nullptr)
{

}

MRenderGraphNodeInput* MRenderGraphNode::AppendInput()
{
	MRenderGraphNodeInput* pInput = new MRenderGraphNodeInput();
	pInput->pGraphNode = this;
	pInput->m_unIndex = m_vInputTextures.size();
	m_vInputTextures.push_back(pInput);

	return pInput;
}

MRenderGraphNodeOutput* MRenderGraphNode::AppendOutput()
{
	MRenderGraphNodeOutput* pOutput = new MRenderGraphNodeOutput();
	pOutput->pGraphNode = this;
	pOutput->m_unIndex = m_vOutputTextures.size();

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

void MRenderGraphNode::UpdateBuffer(MIDevice* pDevice)
{
	if (m_bDirty)
	{
		m_bDirty = false;

		//chean Renderpass.
		m_pRenderPass->DestroyBuffer(pDevice);
		m_pRenderPass->m_vBackDesc = {};
		MFrameBuffer* pFrameBuffer = m_pRenderPass->GetFrameBuffer();
		pFrameBuffer->vBackTextures = {};
		pFrameBuffer->pDepthTexture = nullptr;

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
		m_pRenderPass->GenerateBuffer(pDevice);
	}
}

void MRenderGraphNode::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pRenderPass)
	{
		m_pRenderPass->DestroyBuffer(pDevice);
	}
}

MRenderGraph::MRenderGraph()
	: m_tGraphNodeMap()
	, m_tGraphTextureMap()
	, m_vSortedNodes()
	, m_vRelationTextures()
	, m_v2OutputSize(256.0f, 256.0f)
	, m_bCompiled(false)
	, m_pEngine(nullptr)
	, m_pFinalOutput(nullptr)
{

}

MRenderGraph::MRenderGraph(MEngine* pEngine)
	: m_tGraphNodeMap()
	, m_tGraphTextureMap()
	, m_vSortedNodes()
	, m_vRelationTextures()
	, m_v2OutputSize(256.0f, 256.0f)
	, m_bCompiled(false)
	, m_pEngine(pEngine)
	, m_pFinalOutput(nullptr)
{

}

MRenderGraph::~MRenderGraph()
{

}

MRenderGraphNode* MRenderGraph::AddRenderGraphNode(const MString& strNodeName)
{
	MString strValidName = strNodeName;

	int i = 0;
	while (FindRenderGraphNode(strValidName))
	{
		strValidName = strNodeName + "_" + MStringHelper::ToString(++i);
	}

	MRenderGraphNode* pGraphNode = NewRenderGraphNode();

	pGraphNode->m_strNodeName = strValidName;
	pGraphNode->m_pGraph = this;

	m_tGraphNodeMap[strValidName] = pGraphNode;

	return pGraphNode;
}

MRenderGraphTexture* MRenderGraph::AddRenderGraphTexture(const MString& strTextureName)
{
	MString strValidName = strTextureName;

	int i = 0;
	while(FindRenderGraphTexture(strValidName))
	{
		strValidName = strTextureName +  "_" + MStringHelper::ToString(++i);
	}

	MRenderGraphTexture* pGraphTexture = new MRenderGraphTexture();
	pGraphTexture->m_strTextureName = strValidName;
	pGraphTexture->m_pGraph = this;

	m_tGraphTextureMap[strValidName] = pGraphTexture;

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

void MRenderGraph::SetFinalOutput(MRenderGraphNodeOutput* pFinalOutput)
{
	if (!pFinalOutput)
	{
		m_pFinalOutput = pFinalOutput;
	}
	else if (MRenderGraphNode* pNode = pFinalOutput->GetRenderGraphNode())
	{
		if (pNode->m_pGraph == this)
		{
			m_pFinalOutput = pFinalOutput;
		}
	}
}

MRenderGraphNodeOutput* MRenderGraph::GetFinalOutput() const
{
	return m_pFinalOutput;
}

MRenderGraphTexture* MRenderGraph::GetFinalOutputTexture() const
{
	if (m_pFinalOutput)
	{
		return m_pFinalOutput->GetRenderTexture();
	}

	return nullptr;
}

void MRenderGraph::AddRelationTexture(MRenderGraphTexture* pTexture)
{
	UNION_PUSH_BACK_VECTOR(m_vRelationTextures, pTexture);
}

void MRenderGraph::RemoveRelationTexture(MRenderGraphTexture* pTexture)
{
	ERASE_FIRST_VECTOR(m_vRelationTextures, pTexture);
}

void MRenderGraph::UpdateTextureSize(MRenderGraphTexture* pTexture)
{
	if (pTexture->GetSizePolicy() == MRenderGraphTexture::ESizePolicy::ERelative)
	{
		Vector2 size;
		size.x = m_v2OutputSize.x * pTexture->GetSize().x;
		size.y = m_v2OutputSize.y * pTexture->GetSize().y;

		if (size != pTexture->GetRenderTexture()->GetSize())
		{
			pTexture->GetRenderTexture()->SetSize(size);
			pTexture->SetDirty();
		}
	}
	else
	{
		if (pTexture->GetSize() != pTexture->GetRenderTexture()->GetSize())
		{
			pTexture->GetRenderTexture()->SetSize(pTexture->GetSize());
			pTexture->SetDirty();
		}
	}
}

void MRenderGraph::SetOutputSize(const Vector2& v2Size)
{
	if (m_v2OutputSize != v2Size)
	{
		m_v2OutputSize = v2Size;

		m_v2OutputSize.x = (std::max)(m_v2OutputSize.x, 1.0f);
		m_v2OutputSize.y = (std::max)(m_v2OutputSize.y, 1.0f);

	}


	for (MRenderGraphTexture* pTexture : m_vRelationTextures)
	{
		UpdateTextureSize(pTexture);
	}
}

void MRenderGraph::CompileDirty()
{
	m_bCompiled = false;
}

bool MRenderGraph::Compile(MIDevice* pDevice)
{
	if (!m_bCompiled)
	{
		MRenderGraphTexture* pFinalTexture = GetFinalOutputTexture();
		if (!pFinalTexture)
		{
			MLogManager::GetInstance()->Warning("RenderGraph FinalTexture == nullptr.");
		}


		const size_t nSize = m_tGraphNodeMap.size();

		m_vSortedNodes.clear();
		for (auto& pr : m_tGraphNodeMap)
		{
			pr.second->m_nCommandLevel = M_INVALID_INDEX;
			m_vSortedNodes.push_back(pr.second);
		}

		std::queue<MRenderGraphNode*> queue;

		for (MRenderGraphNodeOutput* pOutput : pFinalTexture->m_vOutputs)
		{
			queue.push(pOutput->GetRenderGraphNode());
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

		m_bCompiled = true;
	}

	return m_bCompiled;
}

void MRenderGraph::Release()
{
	for (auto& pr : m_tGraphNodeMap)
	{
		pr.second->DestroyBuffer(GetEngine()->GetDevice());
		delete pr.second;
		pr.second = nullptr;
	}

	for (auto& pr : m_tGraphTextureMap)
	{
		pr.second->DestroyBuffer(GetEngine()->GetDevice());
		delete pr.second;
		pr.second = nullptr;
	}

	m_tGraphNodeMap.clear();
	m_tGraphTextureMap.clear();
	m_vSortedNodes.clear();

	m_bCompiled = false;
	m_pFinalOutput = nullptr;
}

void MRenderGraph::Render()
{
	if (!m_bCompiled && !Compile(m_pEngine->GetDevice()))
		return;

	for (MRenderGraphNode* pNode : m_vSortedNodes)
	{
		for (MRenderGraphNodeOutput* pOutput : pNode->m_vOutputTextures)
		{
			pOutput->GetRenderTexture()->UpdateBuffer(GetEngine()->GetDevice());
		}

		pNode->UpdateBuffer(GetEngine()->GetDevice());

		if (pNode->m_funcRender)
		{
			pNode->m_funcRender(pNode);
		}
	}
}

MRenderGraphTexture::MRenderGraphTexture()
	: m_strTextureName()
	, m_pTexture(nullptr)
	, m_eUsage(METextureUsage::ERenderBack)
	, m_eLayout(METextureLayout::ERGBA8)
	, m_eSizePolicy(ESizePolicy::EAbsolute)
	, m_v2Size(256.0f, 256.0f)
	, m_pGraph(nullptr)
{
	m_pTexture = new MRenderTexture();
	m_pTexture->SetUsage(m_eUsage);
	m_pTexture->SetType(m_eLayout);
	m_pTexture->SetSize(Vector2(256.0f, 256.0f));
}

void MRenderGraphTexture::SetSizePolicy(const ESizePolicy& ePolicy)
{
	if (m_eSizePolicy != ePolicy)
	{
		m_eSizePolicy = ePolicy;

		if (m_pGraph)
		{
			if (ESizePolicy::ERelative == m_eSizePolicy)
			{
				m_pGraph->AddRelationTexture(this);
				m_pGraph->UpdateTextureSize(this);
			}
			else
			{
				m_pGraph->RemoveRelationTexture(this);
				m_pGraph->UpdateTextureSize(this);
			}
		}
	}
}

void MRenderGraphTexture::SetUsage(const METextureUsage& eUsage)
{
	if (m_eUsage != eUsage)
	{
		m_eUsage = eUsage;
		m_pTexture->SetUsage(eUsage);
		SetDirty();
	}
}

void MRenderGraphTexture::SetLayout(const METextureLayout& eLayout)
{
	if (m_eLayout != eLayout)
	{
		m_eLayout = eLayout;
		m_pTexture->SetType(eLayout);
		SetDirty();
	}
}

void MRenderGraphTexture::SetSize(const Vector2& size)
{
	if (m_v2Size != size)
	{
		m_v2Size = size;

		if (m_pGraph)
		{
			m_pGraph->UpdateTextureSize(this);
		}
	}
}

Vector2 MRenderGraphTexture::GetOutputSize() const
{
	if (m_pTexture)
	{
		return m_pTexture->GetSize();
	}

	return Vector2(256.0f, 256.0f);
}

void MRenderGraphTexture::SetDirty()
{
	if (!m_bDirty)
	{
		m_bDirty = true;
		for (MRenderGraphNodeOutput* pOutput : m_vOutputs)
		{
			if (MRenderGraphNode* pOwnerNode = pOutput->GetRenderGraphNode())
			{
				pOwnerNode->SetDirty();
			}
		}
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

MRenderGraphNodeOutput* MRenderGraphTexture::GetFinalNodeOutput() const
{
	if (m_vOutputs.empty())
		return nullptr;

	//TODO already compiled.

	MRenderGraphNodeOutput* pResultOutput = m_vOutputs[0];

	for (MRenderGraphNodeOutput* p : m_vOutputs)
	{
		if (MRenderGraphNode* pNode = p->GetRenderGraphNode())
		{
			if (pResultOutput->GetRenderGraphNode()->GetLevel() > pNode->GetLevel())
			{
				pResultOutput = p;
			}
		}
	}

	return pResultOutput;
}

MIRenderTexture* MRenderGraphTexture::GetRenderTexture()
{
	return m_pTexture;
}

void MRenderGraphTexture::UpdateBuffer(MIDevice* pDevice)
{
	if (m_bDirty)
	{
		m_bDirty = false;
		m_pTexture->DestroyBuffer(pDevice);
		m_pTexture->GenerateBuffer(pDevice);
	}
}

void MRenderGraphTexture::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pTexture)
	{
		m_pTexture->DestroyBuffer(pDevice);
	}
}

MRenderGraphNodeOutput::MRenderGraphNodeOutput()
	: m_unIndex(0)
	, bClear(true)
	, mClearColor(MColor::Black_T)
	, pGraphNode(nullptr)
	, pGraphTexture(nullptr)
	, vLinkedInput()
{

}

MString MRenderGraphNodeOutput::GetStringID() const
{
	if (!GetRenderGraphNode())
		return "";

	return GetRenderGraphNode()->GetNodeName() + "_Output_" + MStringHelper::ToString(m_unIndex);
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

MString MRenderGraphNodeInput::GetStringID() const
{
	if (!GetRenderGraphNode())
		return "";

	return GetRenderGraphNode()->GetNodeName() + "_Input_" + MStringHelper::ToString(m_unIndex);
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
