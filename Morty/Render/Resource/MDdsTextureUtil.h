/**
 * @File         MDdsTextureUtil
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MTextureResource.h"

namespace morty
{

struct MTextureResourceData;

class MORTY_API MDdsTextureUtil
{
public:
    static std::unique_ptr<MResourceData> ImportDdsTexture(const MString& strResourcePath);
};

}// namespace morty