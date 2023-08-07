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

#include <map>

class MORTY_API MTextureConverter
{
public:
	MTextureConverter(MEngine* pEngine);
    ~MTextureConverter();


    static std::shared_ptr<MTextureResource> ConvertSingleChannel(std::shared_ptr<MTextureResource> pTexture, size_t nChannel);

};
