#include "MRenderTaskNodeOutput.h"
#include "MRenderTaskNode.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeOutput, MTaskNodeOutput)

void MRenderTaskNodeOutput::SetRenderTarget(MRenderTaskTarget* pRenderTarget) { m_renderTaskTarget = pRenderTarget; }

MTexturePtr MRenderTaskNodeOutput::GetTexture() const
{
    if (m_renderTaskTarget) { return m_renderTaskTarget->GetTexture(); }

    return nullptr;
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::GetOutputDesc() const
{
    const size_t nIndex = GetIndex();
    return GetTaskNode()->DynamicCast<MRenderTaskNode>()->InitOutputDesc().at(nIndex);
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::Create(const METextureFormat& format, const MPassTargetDescription& rtDesc)
{
    return {
            .texture      = MTexture::CreateRenderTarget("", format),
            .renderDesc   = rtDesc,
            .allocPolicy  = MEAllocPolicy::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Scale,
            .scale        = 1.0f,
    };
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::CreateFromInput(const MPassTargetDescription& rtDesc, size_t nInputIdx)
{
    return {
            .texture     = MTextureDesc(),
            .renderDesc  = rtDesc,
            .allocPolicy = MEAllocPolicy::Input,
            .inputIdx    = nInputIdx,
    };
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::Create(
        const MTextureDesc&           texDesc,
        const MPassTargetDescription& rtDesc,
        float                         scale,
        size_t                        texelSize
)
{
    return {
            .texture      = texDesc,
            .renderDesc   = rtDesc,
            .allocPolicy  = MEAllocPolicy::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Scale,
            .scale        = scale,
            .texelSize    = texelSize,
    };
}
MRenderTaskOutputDesc MRenderTaskNodeOutput::CreateFixed(
        const METextureFormat&        format,
        const MPassTargetDescription& rtDesc,
        const Vector2i&               size
)
{
    return {
            .texture      = MTexture::CreateRenderTarget("", format).InitSize(size),
            .renderDesc   = rtDesc,
            .allocPolicy  = MEAllocPolicy::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Fixed,
    };
}
METextureFormat MRenderTaskNodeOutput::GetFormat() const {}
