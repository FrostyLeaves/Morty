#include "MRenderGraph.h"

#include "MEngine.h"
#include "MFunction.h"
#include "MRenderPass.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"


#include <queue>
#include <functional>

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
	MRenderGraphNode* pNode = new MRenderGraphNode();

	if (!AddRenderGraphNode(pNode, strNodeName))
	{
		delete pNode;
		pNode = nullptr;
	}

	return pNode;
}

bool MRenderGraph::AddRenderGraphNode(MRenderGraphNode* pGraphNode, const MString& strNodeName)
{
	if (!pGraphNode)
		return false;

	MString strValidName = strNodeName;

	int i = 0;
	while (FindRenderGraphNode(strValidName))
	{
		strValidName = strNodeName + "_" + MStringHelper::ToString(++i);
	}

	pGraphNode->m_strNodeName = strValidName;
	pGraphNode->m_pGraph = this;

	m_tGraphNodeMap[strValidName] = pGraphNode;

	return true;
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
			pr.second->m_nCommandLevel = MGlobal::M_INVALID_INDEX;
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

		if (pNode->GetDirty())
		{
			pNode->UpdateBuffer(GetEngine()->GetDevice());
			pNode->ResetDirty();
		}

		if (pNode->m_funcUpdate)
		{
			//Update render resources
			pNode->m_funcUpdate(pNode);
		}

		if (pNode->m_funcRender)
		{
			//render
			pNode->m_funcRender(pNode);
		}
	}
}
