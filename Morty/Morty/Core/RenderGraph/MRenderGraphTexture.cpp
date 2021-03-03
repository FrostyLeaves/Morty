#include "MRenderGraphTexture.h"

#include "MEngine.h"
#include "MFunction.h"
#include "MRenderPass.h"
#include "MRenderGraph.h"
#include "MRenderGraphNode.h"


#include <queue>
#include <functional>

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
