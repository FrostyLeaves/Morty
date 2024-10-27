#include "MRenderTaskNodeInput.h"
#include "MRenderTaskNode.h"

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
            .format     = format,
            .allowEmpty = allowEmpty,
            .barrier    = METextureBarrierStage::EPixelShaderWrite,
    };
}

MRenderTaskInputDesc MRenderTaskNodeInput::CreateDepth(const MStringId& name)
{
    return {
            .name       = name,
            .format     = METextureFormat::Depth,
            .allowEmpty = false,
            .barrier    = METextureBarrierStage::EPixelShaderWrite,
    };
}