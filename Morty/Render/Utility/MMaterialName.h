/**
 * @File         MIDevice
 * 
 * @Created      2021-7-7 14:20:55
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MStringId.h"

namespace morty
{

struct MORTY_API MMaterialName {
    //Frame
    static MString FRAME_DEFAULT;

    //Skybox
    static MString SKY_BOX;

    //Deferred
    static MString DEFERRED_LIGHTING;
    static MString DEFERRED_GBUFFER;
    static MString DEFERRED_GBUFFER_SKELETON;

    static MString BASIC_LIGHTING;
    static MString BASIC_LIGHTING_SKELETON;

    static MString SHADOW_MAP;

    static MString SHADOW_MAP_SKELETON;
};

}// namespace morty