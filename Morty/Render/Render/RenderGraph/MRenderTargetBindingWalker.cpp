#include "MRenderTargetBindingWalker.h"

#include "Basic/MTexture.h"
#include "Material/MMaterial.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MUtils.h"

namespace morty
{

class MRenderTargetCacheQueue
{

public:
    static MHashCode Hash(const MRenderTaskOutputDesc& desc)
    {
        MHashCode res = 0;
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
            auto texture = MTexture::CreateTexture(desc.texture);
            texture->GenerateBuffer(pDevice);
            m_allTextures.push_back(texture);
            return texture;
        }

        auto texture = m_renderTargetCache[hash].front();
        m_renderTargetCache[hash].pop();

        return texture;
    }

    void RecoveryTexture(const MRenderTaskOutputDesc& desc, const MTexturePtr& texture)
    {
        const size_t hash = Hash(desc);

        m_renderTargetCache[hash].push(texture);
    }

    void Release(MIDevice* pDevice)
    {
        for (auto& texture: m_allTextures)
        {
            if (texture) { texture->DestroyBuffer(pDevice); }
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

MRenderTargetBindingWalker::MRenderTargetBindingWalker(MEngine* engine)
    : m_engine(engine)
    , m_cacheQueue(new MRenderTargetCacheQueue())
{}

MRenderTargetBindingWalker::~MRenderTargetBindingWalker()
{
    const MRenderSystem* renderSystem = m_engine->FindSystem<MRenderSystem>();
    m_cacheQueue->Release(renderSystem->GetDevice());
    MORTY_SAFE_DELETE(m_cacheQueue);

    for (const auto& texture: m_exclusiveTextures) { texture->DestroyBuffer(renderSystem->GetDevice()); }
    m_exclusiveTextures.clear();
}

void MRenderTargetBindingWalker::SetForceExclusive(bool bForce) { m_forceExclusive = bForce; }

void MRenderTargetBindingWalker::operator()(MTaskGraph* pTaskGraph)
{
    if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile()) { return; }

    auto pRenderGraph = pTaskGraph->DynamicCast<MRenderGraph>();
    if (nullptr == pRenderGraph) { return; }

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

            if (pPrevProcessNode && IsAllNextNodeHasAlloced(pPrevProcessNode)) { FreeRenderTarget(pPrevProcessNode); }
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
        if (pRenderNode && pRenderNode->IsValidRenderNode())
        {
            pRenderNode->BindInOutTexture();
            pRenderNode->RegisterSetting();
            pRenderNode->Resize(pRenderGraph->GetSize());
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

    const MRenderSystem* renderSystem = m_engine->FindSystem<MRenderSystem>();
    const auto&          desc         = pOutput->GetOutputDesc();

    MORTY_ASSERT(desc.allocPolicy != METextureSourceType::Input);

    if (desc.sharedPolicy == MESharedPolicy::Exclusive || m_forceExclusive)
    {
        auto texture = MTexture::CreateTexture(desc.texture);
        texture->GenerateBuffer(renderSystem->GetDevice());
        pOutput->SetRenderTexture(texture);

        m_exclusiveTextures.push_back(texture);
    }
    else if (desc.sharedPolicy == MESharedPolicy::Shared)
    {
        if (m_targetAllocCount.find(pOutput) == m_targetAllocCount.end())
        {
            auto texture = m_cacheQueue->AllocTexture(desc, renderSystem->GetDevice());
            pOutput->SetRenderTexture(texture);

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
            auto texture = pOutput->GetRenderTexture();
            m_cacheQueue->RecoveryTexture(desc, texture);

            m_targetAllocCount.erase(findCount);
        }
        else { findCount->second--; }
    }
}

bool MRenderTargetBindingWalker::IsAllPrevNodeHasAlloced(MRenderTaskNode* pNode)
{
    for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
    {
        auto pInput    = static_cast<MRenderTaskNodeInput*>(pNode->GetInput(nInputIdx));
        auto pPrevNode = pInput->GetLinkedNode();
        if (!pInput->GetInputDesc().allowEmpty && m_allocedNode.find(pPrevNode) == m_allocedNode.end())
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
