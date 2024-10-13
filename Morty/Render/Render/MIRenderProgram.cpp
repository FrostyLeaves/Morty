#include "Render/MIRenderProgram.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MIRenderProgram, MObject)

MTextureArray MIRenderProgram::GetOutputTextures()
{
    if (MTexturePtr pTexture = GetOutputTexture()) { return {pTexture}; }

    return {};
}
