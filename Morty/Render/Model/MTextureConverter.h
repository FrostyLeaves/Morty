/**
 * @File         MTextureConverter
 * 
 * @Created      2023-03-13 22:42:07
 *
 * @Author       DoubleYe
 *
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MTextureResource.h"

namespace morty
{

class MORTY_API MTextureConverter
{
public:
    static void ConvertSingleChannel(MTextureResourceData* pTexture, size_t nChannel);
};

}// namespace morty