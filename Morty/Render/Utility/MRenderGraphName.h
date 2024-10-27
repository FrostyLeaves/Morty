#pragma once


#include "Utility/MGlobal.h"
#include "Utility/MStringId.h"

namespace morty
{

struct MORTY_API MRenderGraphName {

    static const MStringId ColorBuffer;
    static const MStringId DepthBuffer;
    static const MStringId EdgeDetection;
    static const MStringId ShadowMap;
    static const MStringId ToneMapping;
    static const MStringId Voxelizer;
    static const MStringId TextureVRS;
    static const MStringId TextureAO;
    static const MStringId TransparentFront;
    static const MStringId TransparentBack;

    static const MStringId GBuffer[3];
};

}// namespace morty