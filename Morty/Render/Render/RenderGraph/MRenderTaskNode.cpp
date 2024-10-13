#include "MRenderTaskNode.h"

#include "MRenderGraph.h"
#include "MRenderTaskNodeInput.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNode, MTaskNode)

void MRenderTaskNode::OnCreated()
{
    for (const auto& input: InitInputDesc())
    {
        auto pInput = AppendInput<MRenderTaskNodeInput>();
        pInput->SetName(input.name);
        m_input[input.name] = pInput;
    }

    for (const auto& output: InitOutputDesc())
    {
        auto pOutput = AppendOutput<MRenderTaskNodeOutput>();
        pOutput->SetName(output.name);
        m_output[output.name] = pOutput;
    }

    SetThreadType(METhreadType::ERenderThread);
}

void                  MRenderTaskNode::OnDelete() { Release(); }

MRenderGraph*         MRenderTaskNode::GetRenderGraph() const { return GetGraph()->DynamicCast<MRenderGraph>(); }

MRenderTargetManager* MRenderTaskNode::GetRenderTargetManager() const
{
    return GetRenderGraph()->GetRenderTargetManager();
}

MTexturePtr MRenderTaskNode::GetInputTexture(const size_t& nIdx)
{
    return GetInput(nIdx)->GetLinkedOutput()->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MTexturePtr MRenderTaskNode::GetOutputTexture(const size_t& nIdx)
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MTexturePtr MRenderTaskNode::GetInputTexture(const MStringId& nIdx)
{
    auto findResult = m_input.find(nIdx);
    if (findResult == m_input.end()) { return nullptr; }

    return findResult->second->GetLinkedOutput()->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MTexturePtr MRenderTaskNode::GetOutputTexture(const MStringId& nIdx)
{
    auto findResult = m_output.find(nIdx);
    if (findResult == m_output.end()) { return nullptr; }

    return findResult->second->GetTexture();
}

MRenderTaskNodeOutput* MRenderTaskNode::GetRenderOutput(const size_t& nIdx)
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>();
}

void               MRenderTaskTarget::SetTexture(const MTexturePtr& pTexture) { m_texture = pTexture; }

MRenderTaskTarget* MRenderTaskTarget::InitName(const MStringId& name)
{
    m_name = name;
    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitResizePolicy(MEResizePolicy ePolicy, float fScale, size_t nTexelSize)
{
    m_resizePolicy = ePolicy;
    m_scale        = fScale;
    m_texelSize    = nTexelSize;

    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitSharedPolicy(MESharedPolicy ePolicy)
{
    m_sharedPolicy = ePolicy;
    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitTextureDesc(const MTextureDesc& desc)
{
    m_textureDesc = desc;
    return this;
}
