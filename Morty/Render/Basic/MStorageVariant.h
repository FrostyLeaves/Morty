/**
 * @File         MVariant
 *
 * @Created      2019-09-01 02:09:49
 *
 * @Author       DoubleYe
 *
 * Only For Shader.
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Utility/MString.h"

namespace morty
{

struct MORTY_API MStorageVariant {
public:
    [[nodiscard]] size_t GetSize() const { return data.size(); }

    void                 ResizeMemory(MIDevice* pDevice, size_t nSize);

    void                 UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* pData, const size_t& nSize);

    std::vector<MByte>   data;
    MBuffer              buffer;
};

}// namespace morty