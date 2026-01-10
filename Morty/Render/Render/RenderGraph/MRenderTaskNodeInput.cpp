#include "MRenderTaskNodeInput.h"
#include "MRenderTaskNode.h"
#include "Utility/MUtils.h"
#include "MRenderTaskNodeOutput.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeInput, MTaskNodeInput)

void MRenderTaskNodeInput::SetInputDesc(const MRenderTaskInputDesc& desc)
{
    m_desc = desc;
    SetName(desc.name);
}

const MTypeClass* MRenderTaskNodeInput::GetDataInternal() const
{
    auto output = GetLinkedOutput();
    return output ? static_cast<MRenderTaskNodeOutput*>(output)->GetDataInternal() : nullptr;
}

MRenderTaskInputDesc MRenderTaskNodeInput::CreateSample(const MStringId& name, METextureFormat format, bool allowEmpty)
{
    return {
            .name       = name,
            .type       = MRenderNodeOutputType::RenderTarget,
            .format     = format,
            .allowEmpty = allowEmpty,
            .barrier    = METextureBarrierStage::EPixelShaderSample,
    };
}
MRenderTaskInputDesc
MRenderTaskNodeInput::CreatePixelWrite(const MStringId& name, METextureFormat format, bool allowEmpty)
{
    return {
            .name       = name,
            .type       = MRenderNodeOutputType::RenderTarget,
            .format     = format,
            .allowEmpty = allowEmpty,
            .barrier    = METextureBarrierStage::EPixelShaderWrite,
    };
}

MRenderTaskInputDesc MRenderTaskNodeInput::CreateDepth(const MStringId& name)
{
    return {
            .name       = name,
            .type       = MRenderNodeOutputType::RenderTarget,
            .format     = METextureFormat::Depth,
            .allowEmpty = false,
            .barrier    = METextureBarrierStage::EPixelShaderWrite,
    };
}

MRenderTaskInputDesc MRenderTaskNodeInput::CreateData(const MStringId& name, const MType* dataType)
{
    return {
            .name     = name,
            .type     = MRenderNodeOutputType::Data,
            .dataType = dataType,
    };
}

MHashCode MRenderTaskInputDesc::GetLinkHash() const
{
    MHashCode hash = 0;

    MUtils::HashCombine(hash, type);
    MUtils::HashCombine(hash, format);
    MUtils::HashCombine(hash, dataType);

    return hash;
}