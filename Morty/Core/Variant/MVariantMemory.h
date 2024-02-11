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
#include "Utility/MString.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

MORTY_SPACE_BEGIN

class MORTY_API MVariantMemory
{
public:
	MVariantMemory() = default;

	size_t AllocMemory(size_t nMemorySize);

	void ByteAlignment();

	size_t Size() const { return m_nMemorySize; }
	MByte* Data() { return m_vMemory.data(); }

public:

	MVariantMemory& operator=(const MVariantMemory& other);

public:
	std::vector<MByte> m_vMemory;
	size_t m_nMemorySize = 0;
};

MORTY_SPACE_END