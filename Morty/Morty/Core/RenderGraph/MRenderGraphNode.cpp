#include "MRenderGraphNode.h"

#include "MEngine.h"
#include "MFunction.h"
#include "MRenderPass.h"
#include "MRenderGraph.h"
#include "MRenderGraphTexture.h"

#include <queue>
#include <functional>

MTypedClassImplement(MRenderGraphNode, MTypedClass)

MRenderGraphNode::MRenderGraphNode()
	: m_strNodeName("")
	, m_vInputTextures()
	, m_vOutputTextures()
	, m_nCommandLevel(MGlobal::M_INVALID_INDEX)
	, m_pGraph(nullptr)
	, m_pRenderPass(new MRenderPass())
	, m_bDirty(true)
	, m_funcUpdate(nullptr)
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

void MRenderGraphNode::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pRenderPass)
	{
		m_pRenderPass->DestroyBuffer(pDevice);
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
