#include "MRenderTaskNodeOutput.h"
#include "Basic/MBuffer.h"
#include "MRenderTaskNode.h"
#include "Utility/MUtils.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeOutput, MTaskNodeOutput)

MRenderTaskOutputDesc MRenderTaskNodeOutput::Create(
        const MStringId&              name,
        const METextureFormat&        format,
        const MPassTargetDescription& rtDesc
)
{
    return {
            .name         = name,
            .type         = MRenderNodeOutputType::RenderTarget,
            .texture      = MTexture::CreateRenderTarget("", format),
            .renderDesc   = rtDesc,
            .allocPolicy  = METextureSourceType::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Scale,
            .scale        = 1.0f,
    };
}

MRenderTaskOutputDesc
MRenderTaskNodeOutput::Create(const MStringId& name, const MTextureDesc& desc, const MPassTargetDescription& rtDesc)
{
    return {
            .name         = name,
            .type         = MRenderNodeOutputType::RenderTarget,
            .texture      = desc,
            .renderDesc   = rtDesc,
            .allocPolicy  = METextureSourceType::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Scale,
            .scale        = 1.0f,
    };
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::CreateDepth(const MStringId& name, const MPassTargetDescription& rtDesc)
{
    return {
            .name         = name,
            .type         = MRenderNodeOutputType::RenderTarget,
            .texture      = MTexture::CreateDepthBuffer(""),
            .renderDesc   = rtDesc,
            .allocPolicy  = METextureSourceType::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Scale,
            .scale        = 1.0f,
    };
}

MRenderTaskOutputDesc
MRenderTaskNodeOutput::CreateFromInput(const MStringId& name, const MPassTargetDescription& rtDesc, size_t nInputIdx)
{
    return {
            .name        = name,
            .type        = MRenderNodeOutputType::RenderTarget,
            .texture     = MTextureDesc(),
            .renderDesc  = rtDesc,
            .allocPolicy = METextureSourceType::Input,
            .inputIdx    = nInputIdx,
    };
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::Create(
        const MStringId&              name,
        const MTextureDesc&           texDesc,
        const MPassTargetDescription& rtDesc,
        float                         scale,
        size_t                        texelSize
)
{
    return {
            .name         = name,
            .type         = MRenderNodeOutputType::RenderTarget,
            .texture      = texDesc,
            .renderDesc   = rtDesc,
            .allocPolicy  = METextureSourceType::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Scale,
            .scale        = scale,
            .texelSize    = texelSize,
    };
}
MRenderTaskOutputDesc MRenderTaskNodeOutput::CreateFixed(
        const MStringId&              name,
        const METextureFormat&        format,
        const MPassTargetDescription& rtDesc,
        const Vector2i&               size
)
{
    return {
            .name         = name,
            .type         = MRenderNodeOutputType::RenderTarget,
            .texture      = MTexture::CreateRenderTarget("", format).InitSize(size),
            .renderDesc   = rtDesc,
            .allocPolicy  = METextureSourceType::Allocate,
            .sharedPolicy = MESharedPolicy::Exclusive,
            .resizePolicy = MEResizePolicy::Fixed,
    };
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::CreateBuffer(const MStringId& name)
{
    return {.name = name, .type = MRenderNodeOutputType::Data, .dataType = MBuffer::GetClassType()};
}

MRenderTaskOutputDesc MRenderTaskNodeOutput::CreateData(const MStringId& name, const MType* dataType)
{
    return {.name = name, .type = MRenderNodeOutputType::Data, .dataType = dataType};
}

METextureFormat MRenderTaskNodeOutput::GetFormat() const
{
    if (m_desc.allocPolicy == METextureSourceType::Input)
    {
        auto input = static_cast<MRenderTaskNode*>(GetTaskNode())->GetInput(m_desc.inputIdx);
        return static_cast<MRenderTaskNodeInput*>(input)->GetInputDesc().format;
    }

    return m_desc.texture.eFormat;
}

MRenderTaskNodeOutput* MRenderTaskNodeOutput::GetActualOutput()
{
    if (m_desc.allocPolicy == METextureSourceType::Input)
    {
        auto input      = static_cast<MRenderTaskNode*>(GetTaskNode())->GetInput(m_desc.inputIdx);
        auto prevOutput = static_cast<MRenderTaskNodeOutput*>(input->GetLinkedOutput());
        if (!prevOutput) { return nullptr; }

        return prevOutput->GetActualOutput();
    }

    return this;
}

bool MRenderTaskNodeOutput::CanLink(const MTaskNodeInput* input) const
{
    auto renderInput = input->DynamicCast<MRenderTaskNodeInput>();
    if (!renderInput) return false;

    return renderInput->GetInputDesc().GetLinkHash() == m_desc.GetLinkHash();
}

void MRenderTaskNodeOutput::SetOutputDesc(const MRenderTaskOutputDesc& desc)
{
    m_desc = desc;
    if (m_desc.texture.strName.empty())
    {
        m_desc.texture.strName = GetTaskNode()->GetNodeName().ToString() + "_out_" + MStringUtil::ToString(GetIndex());
    }

    SetName(m_desc.name);
}

MRenderTarget MRenderTaskNodeOutput::CreateRenderTarget() { return {GetActualTexture(), m_desc.renderDesc}; }

MHashCode     MRenderTaskOutputDesc::GetLinkHash() const
{
    MHashCode hash = 0;

    MUtils::HashCombine(hash, type);
    MUtils::HashCombine(hash, texture.eFormat);
    MUtils::HashCombine(hash, dataType);

    return hash;
}