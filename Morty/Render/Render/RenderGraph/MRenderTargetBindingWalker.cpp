#include "MRenderTargetBindingWalker.h"

#include "Basic/MTexture.h"
#include "Material/MMaterial.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MUtils.h"

namespace morty
{

class MRenderTargetCacheQueue
{

public:
    static size_t Hash(const MRenderTaskOutputDesc& desc)
    {
        std::size_t res = 0;
        if (desc.resizePolicy == MEResizePolicy::Fixed)
        {
            MUtils::HashCombine(res, desc.texture.n3Size.x);
            MUtils::HashCombine(res, desc.texture.n3Size.y);
            MUtils::HashCombine(res, desc.texture.n3Size.z);
        }
        else
        {
            MUtils::HashCombine(res, static_cast<int>(desc.scale * 100));
            MUtils::HashCombine(res, desc.texelSize);
        }
        MUtils::HashCombine(res, desc.texture.eTextureType);
        MUtils::HashCombine(res, desc.texture.eFormat);
        MUtils::HashCombine(res, desc.texture.nWriteUsage);
        MUtils::HashCombine(res, desc.texture.nReadUsage);
        MUtils::HashCombine(res, desc.texture.eMipmapDataType);

        return res;
    }

    MTexturePtr AllocTexture(const MRenderTaskOutputDesc& desc, MIDevice* pDevice)
    {
        const size_t hash = Hash(desc);

        if (m_renderTargetCache[hash].empty())
        {
            auto pTexture = MTexture::CreateTexture(desc.texture);
            pTexture->GenerateBuffer(pDevice);
            m_allTextures.push_back(pTexture);
            return pTexture;
        }

        auto pTexture = m_renderTargetCache[hash].front();
        m_renderTargetCache[hash].pop();

        return pTexture;
    }

    void RecoveryTexture(const MRenderTaskOutputDesc& desc, const MTexturePtr& pTexture)
    {
        const size_t hash = Hash(desc);

        m_renderTargetCache[hash].push(pTexture);
    }

    void Release(MIDevice* pDevice)
    {
        for (auto& pTexture: m_allTextures)
        {
            if (pTexture) { pTexture->DestroyBuffer(pDevice); }
        }

        m_renderTargetCache.clear();
        m_allTextures.clear();
    }

private:
    std::map<size_t, std::queue<MTexturePtr>> m_renderTargetCache;

    MTextureArray                             m_allTextures;
};

}// namespace morty

using namespace morty;

MRenderTargetBindingWalker::MRenderTargetBindingWalker(MEngine* pEngine)
    : m_engine(pEngine)
    , m_cacheQueue(new MRenderTargetCacheQueue())
{}

MRenderTargetBindingWalker::~MRenderTargetBindingWalker()
{
    const MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    m_cacheQueue->Release(pRenderSystem->GetDevice());
    MORTY_SAFE_DELETE(m_cacheQueue);

    for (const auto& pTexture: m_exclusiveTextures) { pTexture->DestroyBuffer(pRenderSystem->GetDevice()); }
    m_exclusiveTextures.clear();
}

void MRenderTargetBindingWalker::SetForceExclusive(bool bForce) { m_forceExclusive = bForce; }

void MRenderTargetBindingWalker::operator()(MTaskGraph* pTaskGraph)
{
    if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile()) { return; }

    std::vector<MTaskNode*> vNodeStack = pTaskGraph->GetStartNodes();


    while (!vNodeStack.empty())
    {
        auto* pCurrentNode = vNodeStack.back()->DynamicCast<MRenderTaskNode>();
        vNodeStack.pop_back();

        AllocRenderTarget(pCurrentNode);

        for (size_t nInputIdx = 0; nInputIdx < pCurrentNode->GetInputSize(); ++nInputIdx)
        {
            auto pInput           = pCurrentNode->GetInput(nInputIdx);
            auto pPrevProcessNode = pInput->GetLinkedNode()->DynamicCast<MRenderTaskNode>();

            if (IsAllNextNodeHasAlloced(pPrevProcessNode)) { FreeRenderTarget(pPrevProcessNode); }
        }

        for (size_t nOutputIdx = 0; nOutputIdx < pCurrentNode->GetOutputSize(); ++nOutputIdx)
        {
            auto pOutput = pCurrentNode->GetOutput(nOutputIdx);

            for (auto pInput: pOutput->GetLinkedInputs())
            {
                auto pNextNode = pInput->GetTaskNode()->DynamicCast<MRenderTaskNode>();

                if (pNextNode && IsAllPrevNodeHasAlloced(pNextNode)) { vNodeStack.push_back(pNextNode); }
            }
        }
    }


    for (MTaskNode* pNode: pTaskGraph->GetAllNodes())
    {
        auto pRenderNode = pNode->DynamicCast<MRenderTaskNode>();
        if (pRenderNode)
        {
            pRenderNode->BindInOutTexture();
            pRenderNode->RegisterSetting();
        }
    }
}

void MRenderTargetBindingWalker::AllocRenderTarget(MRenderTaskNode* pNode)
{
    if (m_allocedNode.find(pNode) != m_allocedNode.end()) { return; }

    m_allocedNode[pNode] = AllocState::Alloced;

    for (size_t nIdx = 0; nIdx < pNode->GetOutputSize(); ++nIdx)
    {
        auto pOutput = pNode->GetRenderOutput(nIdx);

        AllocRenderTarget(pOutput->GetActualOutput());
    }
}

void MRenderTargetBindingWalker::FreeRenderTarget(MRenderTaskNode* pNode)
{
    if (m_allocedNode.find(pNode) == m_allocedNode.end()) { return; }

    for (size_t nIdx = 0; nIdx < pNode->GetOutputSize(); ++nIdx)
    {
        auto pOutput = pNode->GetRenderOutput(nIdx);

        FreeRenderTarget(pOutput);
    }

    m_allocedNode[pNode] = AllocState::Free;
}

void MRenderTargetBindingWalker::AllocRenderTarget(MRenderTaskNodeOutput* pOutput)
{
    if (pOutput == nullptr) return;

    const MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    const auto&          desc          = pOutput->GetOutputDesc();

    MORTY_ASSERT(desc.allocPolicy != METextureSourceType::Input);

    if (desc.sharedPolicy == MESharedPolicy::Exclusive || m_forceExclusive)
    {
        auto pTexture = MTexture::CreateTexture(desc.texture);
        pTexture->GenerateBuffer(pRenderSystem->GetDevice());
        pOutput->SetRenderTexture(pTexture);

        m_exclusiveTextures.push_back(pTexture);
    }
    else if (desc.sharedPolicy == MESharedPolicy::Shared)
    {
        if (m_targetAllocCount.find(pOutput) == m_targetAllocCount.end())
        {
            auto pTexture = m_cacheQueue->AllocTexture(desc, pRenderSystem->GetDevice());
            pOutput->SetRenderTexture(pTexture);

            m_targetAllocCount[pOutput] = 0;
        }
        m_targetAllocCount[pOutput]++;
    }
    else { MORTY_ASSERT(false); }
}

void MRenderTargetBindingWalker::FreeRenderTarget(MRenderTaskNodeOutput* pOutput)
{
    if (pOutput == nullptr) return;

    const auto& desc = pOutput->GetOutputDesc();

    MORTY_ASSERT(desc.allocPolicy != METextureSourceType::Input);

    if (!m_forceExclusive && desc.sharedPolicy == MESharedPolicy::Shared)
    {
        auto findCount = m_targetAllocCount.find(pOutput);
        if (findCount == m_targetAllocCount.end()) { return; }
        if (findCount->second <= 1)
        {
            auto pTexture = pOutput->GetRenderTexture();
            m_cacheQueue->RecoveryTexture(desc, pTexture);

            m_targetAllocCount.erase(findCount);
        }
        else { findCount->second--; }
    }
}

bool MRenderTargetBindingWalker::IsAllPrevNodeHasAlloced(MRenderTaskNode* pNode)
{
    for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
    {
        auto pInput    = pNode->GetInput(nInputIdx);
        auto pPrevNode = pInput->GetLinkedNode();
        if (m_allocedNode.find(pPrevNode) == m_allocedNode.end()) { return false; }
    }

    return true;
}

bool MRenderTargetBindingWalker::IsAllNextNodeHasAlloced(MRenderTaskNode* pNode)
{
    for (size_t nOutputIdx = 0; nOutputIdx < pNode->GetOutputSize(); ++nOutputIdx)
    {
        auto pOutput = pNode->GetOutput(nOutputIdx);
        for (auto pInput: pOutput->GetLinkedInputs())
        {
            auto pNextNode = pInput->GetTaskNode();
            if (m_allocedNode.find(pNextNode) == m_allocedNode.end()) { return false; }
        }
    }

    return true;
}

bool MRenderTargetBindingWalker::IsNodeHasAlloced(MRenderTaskNode* pNode)
{
    if (m_allocedNode.find(pNode) == m_allocedNode.end()) { return false; }

    return true;
}
