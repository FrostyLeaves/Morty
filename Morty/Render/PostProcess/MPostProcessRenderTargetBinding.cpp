#include "MPostProcessRenderTargetBinding.h"

#include "MPostProcessNode.h"
#include "Render/MRenderCommand.h"
#include "Render/MRenderPass.h"
#include "Material/MMaterial.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"


MPostProcessRenderTargetBinding::MPostProcessRenderTargetBinding(MEngine* pEngine)
    : m_pEngine(pEngine)
{
}

void MPostProcessRenderTargetBinding::operator ()(MTaskGraph* pTaskGraph)
{
	if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile())
	{
		return;
	}

	std::vector<MTaskNode*> vNodeStack = pTaskGraph->GetStartNodes();


	while (!vNodeStack.empty())
	{
		MTaskNode* pCurrentNode = vNodeStack.back();
		vNodeStack.pop_back();

		if (!m_pFrameProperty)
		{
			auto pPostProcessNode = pCurrentNode->DynamicCast<MPostProcessNode>();
			auto pMaterial = pPostProcessNode->GetMaterial();
			m_pFrameProperty = MMaterial::CreateFramePropertyBlock(pMaterial->GetShaderProgram());
		}

		AllocRenderTarget(pCurrentNode->DynamicCast<MPostProcessNode>());

		for (size_t nInputIdx = 0; nInputIdx < pCurrentNode->GetInputSize(); ++nInputIdx)
		{
			auto pInput = pCurrentNode->GetInput(nInputIdx);
			auto pPrevNode = pInput->GetLinkedNode();

			if (IsAllNextNodeHasAlloced(pPrevNode->DynamicCast<MPostProcessNode>()))
			{
				auto pPrevProcessNode = pPrevNode->DynamicCast<MPostProcessNode>();
				auto pPrevRenderPass = pPrevProcessNode->GetRenderPass();

				MORTY_ASSERT(nInputIdx < pPrevRenderPass->GetBackTextures().size());
				auto pTexture = pPrevRenderPass->GetBackTextures()[nInputIdx];

				auto pPostProcessNode = pCurrentNode->DynamicCast<MPostProcessNode>();
				auto pMaterial = pPostProcessNode->GetMaterial();
				auto pPropertyBlock = pMaterial->GetMaterialPropertyBlock();

				MORTY_ASSERT(nInputIdx < pPropertyBlock->m_vTextures.size());
				pPropertyBlock->m_vTextures[nInputIdx]->pTexture = pTexture;
				pPropertyBlock->m_vTextures[nInputIdx]->SetDirty();

				FreeRenderTarget(pPrevNode->DynamicCast<MPostProcessNode>());
			}
		}

		for (size_t nOutputIdx = 0; nOutputIdx < pCurrentNode->GetOutputSize(); ++nOutputIdx)
		{
			auto pOutput = pCurrentNode->GetOutput(nOutputIdx);

			for (auto pInput : pOutput->GetLinkedInputs())
			{
				auto pNextNode = pInput->GetTaskNode()->DynamicCast<MPostProcessNode>();

				if (pNextNode && IsAllPrevNodeHasAlloced(pNextNode))
				{
					vNodeStack.push_back(pNextNode);
				}
			}
		}
	}
}

void MPostProcessRenderTargetBinding::Resize(const Vector2i size)
{
	if (m_n2ScreenSize == size)
	{
		return;
	}

    MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	for (auto pRenderPass : m_vAllRenderPass)
	{
		pRenderSystem->ResizeFrameBuffer(*pRenderPass, size);
	}

	m_n2ScreenSize = size;

	if (m_pFrameProperty)
	{
		m_pFrameProperty->SetValue(MShaderPropertyName::POSTPROCESS_SCREEN_SIZE, Vector2(size.x, size.y));
	}
}

void MPostProcessRenderTargetBinding::Release()
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	for(auto pRenderPass : m_vAllRenderPass)
	{
		pRenderPass->DestroyBuffer(pRenderSystem->GetDevice());
	}

	for (auto pTexture : m_vAllTextures)
	{
		pTexture->DestroyBuffer(pRenderSystem->GetDevice());
	}
}

std::shared_ptr<IPropertyBlockAdapter> MPostProcessRenderTargetBinding::GetFrameProperty() const
{
	class PropertyAdapter : public IPropertyBlockAdapter
	{
	public:
		explicit PropertyAdapter(const std::shared_ptr<MShaderPropertyBlock>& pFrameProperty): m_pFrameProperty(pFrameProperty){}
		std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const override
	    {
			return m_pFrameProperty;
	    }

		std::shared_ptr<MShaderPropertyBlock> m_pFrameProperty = nullptr;
	};

	return std::make_shared<PropertyAdapter>(m_pFrameProperty);
}

void MPostProcessRenderTargetBinding::AllocRenderTarget(MPostProcessNode* pNode)
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	if (m_tAllocedNode.find(pNode) != m_tAllocedNode.end())
	{
		return;
	}

	m_tAllocedNode[pNode] = AllocState::Alloced;

	MRenderPass* pRenderPass = new MRenderPass();
	pNode->SetRenderPass(pRenderPass);
	m_vAllRenderPass.push_back(pRenderPass);

	if (pNode->GetRenderTarget().pTexture)
	{
		pRenderPass->AddBackTexture(pNode->GetRenderTarget());
	}
	else
	{
		if (m_vTextures.empty())
		{
			auto pTexture = MTexture::CreateRenderTarget();
			m_vTextures.push(pTexture);
			m_vAllTextures.push_back(pTexture);

			pTexture->SetSize(Vector2i(256, 256));
			pTexture->GenerateBuffer(pRenderSystem->GetDevice());
		}

		auto pTexture = m_vTextures.front();
		m_vTextures.pop();

		pRenderPass->AddBackTexture({ pTexture , {true, MColor::Black_T } });
	}

	pRenderPass->GenerateBuffer(pRenderSystem->GetDevice());

}

void MPostProcessRenderTargetBinding::FreeRenderTarget(MPostProcessNode* pNode)
{
	auto findResult = m_tAllocedNode.find(pNode);
	if (findResult == m_tAllocedNode.end())
	{
		return;
	}


	if (pNode->GetRenderTarget().pTexture)
	{
		return;
	}

	if (findResult->second == AllocState::Alloced)
	{
		for (size_t nOutputIdx = 0; nOutputIdx < pNode->GetOutputSize(); ++nOutputIdx)
		{
			MRenderPass* pRenderPass = pNode->GetRenderPass();
			auto pTexture = pRenderPass->GetBackTextures()[nOutputIdx];
			m_vTextures.push(pTexture);
		}

		m_tAllocedNode[pNode] = AllocState::Free;
	}
}

bool MPostProcessRenderTargetBinding::IsAllPrevNodeHasAlloced(MPostProcessNode* pNode)
{
	for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
	{
		auto pInput = pNode->GetInput(nInputIdx);
		auto pPrevNode = pInput->GetLinkedNode();
		if (m_tAllocedNode.find(pPrevNode) == m_tAllocedNode.end())
		{
			return false;
		}
	}

	return true;
}

bool MPostProcessRenderTargetBinding::IsAllNextNodeHasAlloced(MPostProcessNode* pNode)
{
	for (size_t nOutputIdx = 0; nOutputIdx < pNode->GetOutputSize(); ++nOutputIdx)
	{
		auto pOutput = pNode->GetOutput(nOutputIdx);
		for (auto pInput : pOutput->GetLinkedInputs())
		{
			auto pNextNode = pInput->GetTaskNode();
			if (m_tAllocedNode.find(pNextNode) == m_tAllocedNode.end())
			{
				return false;
			}
		}
	}

	return true;
}
