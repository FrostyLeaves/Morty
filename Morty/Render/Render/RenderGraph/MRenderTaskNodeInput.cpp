#include "MRenderTaskNodeInput.h"
#include "MRenderTaskNode.h"
#include "Utility/MUtils.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeInput, MTaskNodeInput)

void MRenderTaskNodeInput::SetInputDesc(const MRenderTaskInputDesc& desc)
{
    m_desc = desc;
    SetName(desc.name);
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

MHashCode MRenderTaskInputDesc::GetLinkHash() const
{
    MHashCode hash = 0;

    MUtils::HashCombine(hash, type);
    MUtils::HashCombine(hash, format);
    MUtils::HashCombine(hash, dataType);

    return hash;
}