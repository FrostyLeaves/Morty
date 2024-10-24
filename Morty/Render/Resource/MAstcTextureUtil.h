﻿/**
 * @File         MAstcTextureUtil
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

class MORTY_API MAstcTextureUtil
{
public:
    static std::unique_ptr<MResourceData> ImportAstcTexture(const MString& strResourcePath);

    static morty::METextureFormat         GetTextureFormat(const Vector3i& n3BlockDim);
};

}// namespace morty