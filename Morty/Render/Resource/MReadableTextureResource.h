/**
 * @File         MTextureResource
 *
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
 **/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

namespace morty
{

class MORTY_API MReadableTextureResource : public MTextureResource
{
    MORTY_CLASS(MReadableTextureResource)
public:
    MReadableTextureResource() { m_readable = true; }
};

}// namespace morty