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
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Utility/MString.h"

namespace morty
{

class MORTY_API MVariantMemory
{
public:
    MVariantMemory() = default;

    size_t AllocMemory(size_t nMemorySize);

    void   ByteAlignment();

    size_t Size() const { return m_memorySize; }

    MByte* Data() { return m_memory.data(); }

public:
    MVariantMemory& operator=(const MVariantMemory& other);

public:
    std::vector<MByte> m_memory;
    size_t             m_memorySize = 0;
};

}// namespace morty