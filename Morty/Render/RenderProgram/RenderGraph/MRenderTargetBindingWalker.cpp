#include "MRenderTargetBindingWalker.h"

#include "Utility/MUtils.h"
#include "Basic/MTexture.h"
#include "Render/MRenderCommand.h"
#include "Render/MRenderPass.h"
#include "Material/MMaterial.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"

MORTY_SPACE_BEGIN

class MRenderTargetCacheQueue
{

public:

	static size_t Hash(const MTextureDesc& desc, const MRenderTaskTarget::ResizePolicy eResizePolicy)
	{
		std::size_t res = 0;
		MUtils::HashCombine(res, desc.n3Size.x);
		MUtils::HashCombine(res, desc.n3Size.y);
		MUtils::HashCombine(res, desc.n3Size.z);
		MUtils::HashCombine(res, desc.eTextureType);
		MUtils::HashCombine(res, desc.eFormat);
		MUtils::HashCombine(res, desc.nWriteUsage);
		MUtils::HashCombine(res, desc.nReadUsage);
		MUtils::HashCombine(res, desc.eMipmapDataType);
		MUtils::HashCombine(res, eResizePolicy);

		return res;
	}

	std::shared_ptr<MTexture> AllocTexture(const MTextureDesc& desc, const MRenderTaskTarget::ResizePolicy eResizePolicy, MIDevice* pDevice)
	{
		const size_t hash = Hash(desc, eResizePolicy);

		if (m_tRenderTargetCache[hash].empty())
		{
			auto pTexture = MTexture::CreateTexture(desc);
			pTexture->GenerateBuffer(pDevice);
			m_vAllTextures.push_back(pTexture);
			return pTexture;
		}

		auto pTexture = m_tRenderTargetCache[hash].front();
		m_tRenderTargetCache[hash].pop();

		return pTexture;
	}

	void RecoveryTexture(const MTextureDesc& desc, const MRenderTaskTarget::ResizePolicy eResizePolicy, std::shared_ptr<MTexture> pTexture)
	{
		const size_t hash = Hash(desc, eResizePolicy);

		m_tRenderTargetCache[hash].push(pTexture);
	}

	void Release(MIDevice* pDevice)
	{
	    for (auto& pTexture : m_vAllTextures)
	    {
			if (pTexture)
			{
				pTexture->DestroyBuffer(pDevice);
			}
	    }

		m_tRenderTargetCache.clear();
		m_vAllTextures.clear();
	}

private:

	std::map<size_t, std::queue<std::shared_ptr<MTexture>>> m_tRenderTargetCache;

	std::vector<std::shared_ptr<MTexture>> m_vAllTextures;

};

MORTY_SPACE_END

using namespace morty;

MRenderTargetBindingWalker::MRenderTargetBindingWalker(MEngine* pEngine)
    : m_pEngine(pEngine)
    , m_pCacheQueue(new MRenderTargetCacheQueue())
{
}

MRenderTargetBindingWalker::~MRenderTargetBindingWalker()
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pCacheQueue->Release(pRenderSystem->GetDevice());
	MORTY_SAFE_DELETE(m_pCacheQueue);

	for (auto pTexture : m_vExclusiveTextures)
	{
		pTexture->DestroyBuffer(pRenderSystem->GetDevice());
	}
	m_vExclusiveTextures.clear();
}

void MRenderTargetBindingWalker::operator ()(MTaskGraph* pTaskGraph)
{
	if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile())
	{
		return;
	}

	std::vector<MTaskNode*> vNodeStack = pTaskGraph->GetStartNodes();


	while (!vNodeStack.empty())
	{
		MRenderTaskNode* pCurrentNode = vNodeStack.back()->DynamicCast<MRenderTaskNode>();
		vNodeStack.pop_back();

		AllocRenderTarget(pCurrentNode);

		for (size_t nInputIdx = 0; nInputIdx < pCurrentNode->GetInputSize(); ++nInputIdx)
		{
			auto pInput = pCurrentNode->GetInput(nInputIdx);
			auto pPrevProcessNode = pInput->GetLinkedNode()->DynamicCast<MRenderTaskNode>();

			if (IsAllNextNodeHasAlloced(pPrevProcessNode))
			{
				FreeRenderTarget(pPrevProcessNode);
			}
		}

		for (size_t nOutputIdx = 0; nOutputIdx < pCurrentNode->GetOutputSize(); ++nOutputIdx)
		{
			auto pOutput = pCurrentNode->GetOutput(nOutputIdx);

			for (auto pInput : pOutput->GetLinkedInputs())
			{
				auto pNextNode = pInput->GetTaskNode()->DynamicCast<MRenderTaskNode>();

				if (pNextNode && IsAllPrevNodeHasAlloced(pNextNode))
				{
					vNodeStack.push_back(pNextNode);
				}
			}
		}
	}


	for (MTaskNode* pNode : pTaskGraph->GetAllNodes())
	{
		auto pRenderNode = pNode->DynamicCast<MRenderTaskNode>();
		if (pRenderNode)
		{
			pRenderNode->BindTarget();
			pRenderNode->RegisterSetting();
		}
	}
}

void MRenderTargetBindingWalker::AllocRenderTarget(MRenderTaskNode* pNode)
{
	if (m_tAllocedNode.find(pNode) != m_tAllocedNode.end())
	{
		return;
	}

	m_tAllocedNode[pNode] = AllocState::Alloced;

    for (size_t nIdx = 0; nIdx < pNode->GetOutputSize(); ++nIdx)
    {
        auto pRenderTarget = pNode->GetRenderOutput(nIdx)->GetRenderTarget();

		AllocRenderTarget(pRenderTarget);
    }

}

void MRenderTargetBindingWalker::FreeRenderTarget(MRenderTaskNode* pNode)
{
	if (m_tAllocedNode.find(pNode) == m_tAllocedNode.end())
	{
		return;
	}

	for (size_t nIdx = 0; nIdx < pNode->GetOutputSize(); ++nIdx)
	{
		auto pRenderTarget = pNode->GetRenderOutput(nIdx)->GetRenderTarget();

		FreeRenderTarget(pRenderTarget);
	}

	m_tAllocedNode[pNode] = AllocState::Free;
}

void MRenderTargetBindingWalker::AllocRenderTarget(MRenderTaskTarget* pRenderTarget)
{
	const MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	if (pRenderTarget->GetSharedPolicy() == MRenderTaskTarget::SharedPolicy::Exclusive)
	{
		auto pTexture = MTexture::CreateTexture(pRenderTarget->GetTextureDesc());
		pTexture->GenerateBuffer(pRenderSystem->GetDevice());

		pRenderTarget->SetTexture(pTexture);

		m_vExclusiveTextures.push_back(pTexture);
	}
	else if (pRenderTarget->GetSharedPolicy() == MRenderTaskTarget::SharedPolicy::Shared)
	{
		if (m_tTargetAllocCount.find(pRenderTarget) == m_tTargetAllocCount.end())
		{
			auto pTexture = m_pCacheQueue->AllocTexture(pRenderTarget->GetTextureDesc(), pRenderTarget->GetResizePolicy(), pRenderSystem->GetDevice());
			pRenderTarget->SetTexture(pTexture);

			m_tTargetAllocCount[pRenderTarget] = 1;
		}
        else
        {
			m_tTargetAllocCount[pRenderTarget]++;
        }
	}
	else
	{
		MORTY_ASSERT(false);
	}
}

void MRenderTargetBindingWalker::FreeRenderTarget(MRenderTaskTarget* pRenderTarget)
{
	if (pRenderTarget->GetSharedPolicy() == MRenderTaskTarget::SharedPolicy::Shared)
	{
		auto findCount = m_tTargetAllocCount.find(pRenderTarget);
		if (findCount == m_tTargetAllocCount.end())
		{
			return;
		}
		if (findCount->second <= 1)
		{
		    auto pTexture = pRenderTarget->GetTexture();
			m_pCacheQueue->RecoveryTexture(pRenderTarget->GetTextureDesc(), pRenderTarget->GetResizePolicy(), pTexture);

			m_tTargetAllocCount.erase(findCount);
		}
		else
		{
			findCount->second--;
		}
	}
}

bool MRenderTargetBindingWalker::IsAllPrevNodeHasAlloced(MRenderTaskNode* pNode)
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

bool MRenderTargetBindingWalker::IsAllNextNodeHasAlloced(MRenderTaskNode* pNode)
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

bool MRenderTargetBindingWalker::IsNodeHasAlloced(MRenderTaskNode* pNode)
{
	if (m_tAllocedNode.find(pNode) == m_tAllocedNode.end())
	{
		return false;
	}

	return true;
}
