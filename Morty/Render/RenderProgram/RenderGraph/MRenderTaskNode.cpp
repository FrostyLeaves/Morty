#include "MRenderTaskNode.h"

#include "MRenderGraph.h"
#include "MRenderTaskNodeInput.h"
#include "TaskGraph/MTaskGraph.h"

MORTY_CLASS_IMPLEMENT(MRenderTaskNode, MTaskNode)

void MRenderTaskNode::OnCreated()
{
    for (const auto& input : GetInputName())
    {
        AppendInput<MRenderTaskNodeInput>()->SetName(input);
    }

    for (const auto& output : GetOutputName())
    {
        AppendOutput<MRenderTaskNodeOutput>()->SetName(output.name);
    }

    SetThreadType(METhreadType::ERenderThread);
}

void MRenderTaskNode::OnDelete()
{
    Release();
}

MRenderGraph* MRenderTaskNode::GetRenderGraph() const
{
    return GetGraph()->DynamicCast<MRenderGraph>();
}

MRenderTargetManager* MRenderTaskNode::GetRenderTargetManager() const
{
    return GetRenderGraph()->GetRenderTargetManager();
}

std::shared_ptr<MTexture> MRenderTaskNode::GetInputTexture(const size_t& nIdx)
{
    return GetInput(nIdx)->GetLinkedOutput()->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

std::shared_ptr<MTexture> MRenderTaskNode::GetOutputTexture(const size_t& nIdx)
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>()->GetTexture();
}

MRenderTaskNodeOutput* MRenderTaskNode::GetRenderOutput(const size_t& nIdx)
{
    return GetOutput(nIdx)->DynamicCast<MRenderTaskNodeOutput>();
}

void MRenderTaskTarget::SetTexture(const std::shared_ptr<MTexture>& pTexture)
{
    m_pTexture = pTexture;
}

MRenderTaskTarget* MRenderTaskTarget::InitResizePolicy(ResizePolicy ePolicy, float fScale, size_t nTexelSize)
{
    m_eResizePolicy = ePolicy;
    m_fScale = fScale;
    m_nTexelSize = nTexelSize;

    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitSharedPolicy(SharedPolicy ePolicy)
{
    m_eSharedPolicy = ePolicy;
    return this;
}

MRenderTaskTarget* MRenderTaskTarget::InitTextureDesc(const MTextureDesc& desc)
{
    m_textureDesc = desc;
    return this;
}