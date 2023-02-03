/**
 * @File         MVariant
 * 
 * @Created      2019-09-01 02:09:49
 *
 * @Author       DoubleYe
 *
 * Only For Shader.
**/

#ifndef _M_VARIANT_MEMORY_H_
#define _M_VARIANT_MEMORY_H_
#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include <vector>
#include <map>
#include <unordered_map>  


class MORTY_API MVariantMemory
{
public:
	MVariantMemory() = default;

	template<typename TYPE>
	size_t AppendVariant(const TYPE& value);

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

template<typename TYPE>
inline size_t MVariantMemory::AppendVariant(const TYPE& value)
{
	MORTY_ASSERT(false);
}

template<>
inline size_t MVariantMemory::AppendVariant<bool>(const bool& value)
{
	return AllocMemory(sizeof(int));
}

template<>
inline size_t MVariantMemory::AppendVariant<int>(const int& value)
{
	return AllocMemory(sizeof(int));
}

template<>
inline size_t MVariantMemory::AppendVariant<float>(const float& value)
{
	return AllocMemory(sizeof(float));
}

template<>
inline size_t MVariantMemory::AppendVariant<Vector2>(const Vector2& value)
{
	return AllocMemory(sizeof(Vector2));
}

template<>
inline size_t MVariantMemory::AppendVariant<Vector3>(const Vector3& value)
{
	return AllocMemory(sizeof(Vector3));
}

template<>
inline size_t MVariantMemory::AppendVariant<Vector4>(const Vector4& value)
{
	return AllocMemory(sizeof(Vector4));
}

template<>
inline size_t MVariantMemory::AppendVariant<Matrix3>(const Matrix3& value)
{
	return AllocMemory(sizeof(float) * 4 * 3);
}


template<>
inline size_t MVariantMemory::AppendVariant<Matrix4>(const Matrix4& value)
{
	return AllocMemory(sizeof(float) * 16);
}



#endif
