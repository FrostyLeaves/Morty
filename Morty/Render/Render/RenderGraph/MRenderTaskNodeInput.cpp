#include "MRenderTaskNodeInput.h"
#include "MRenderTaskNode.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTaskNodeInput, MTaskNodeInput)
MRenderTaskInputDesc MRenderTaskNodeInput::CreateSample(METextureFormat format, bool allowEmpty)
{
    return {
            .format     = format,
            .allowEmpty = allowEmpty,
            .barrier    = METextureBarrierStage::EPixelShaderSample,
    };
}
MRenderTaskInputDesc MRenderTaskNodeInput::CreatePixelWrite(METextureFormat format, bool allowEmpty)
{
    return {
            .format     = format,
            .allowEmpty = allowEmpty,
            .barrier    = METextureBarrierStage::EPixelShaderWrite,
    };
}

MRenderTaskInputDesc MRenderTaskNodeInput::CreateDepth()
{
    return {
            .format     = METextureFormat::Depth,
            .allowEmpty = false,
            .barrier    = METextureBarrierStage::EPixelShaderWrite,
    };
}