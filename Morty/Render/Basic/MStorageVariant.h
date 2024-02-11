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
#include "Render/MBuffer.h"

MORTY_SPACE_BEGIN

struct MORTY_API MStorageVariant
{
public:

	size_t GetSize() { return data.size(); }

	void ResizeMemory(MIDevice* pDevice, size_t nSize);
	void UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* pData, const size_t& nSize);

	std::vector<MByte> data;
	MBuffer buffer;
};

MORTY_SPACE_END