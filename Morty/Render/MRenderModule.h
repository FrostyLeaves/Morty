/**
 * @File         MRenderModule
 * 
 * @Created      2021-07-19 11:50:59
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MStringId.h"

namespace morty
{

class MEngine;
class MObject;
class MORTY_API MRenderModule
{
public:
    static bool          Register(MEngine* pEngine);

    static void          OnObjectPostCreate(MObject* pObject);

    static void          RegisterMaterial(MEngine* pEngine);

    static const MString DefaultWhite;
    static const MString DefaultNormal;
    static const MString Default_R8_One;
    static const MString Default_R8_Zero;
    static const MString DefaultAnimationMaterial;
    static const MString NoiseTexture;
};

}// namespace morty