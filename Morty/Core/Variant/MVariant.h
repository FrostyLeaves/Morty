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
#include "MVariantMemory.h"

class MVariantStruct;
class MVariantArray;

enum class MEVariantType
{
	ENone,
	EUInt,
	EInt,
	EFloat,
	EVector2,
	EVector3,
	EVector4,
	EMatrix3,
	EMatrix4,
	EStruct,
	EArray,
};

class MORTY_API MVariant
{
public:

	MVariant() = default;
	~MVariant() = default;

	template<typename TYPE>
	explicit MVariant(const TYPE& value);

	static MVariant Clone(const MVariant& value, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

	MVariant(const MVariant& value) = default;
	MVariant(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset, size_t nSize, MEVariantType eType);

	template<typename TYPE>
	static MEVariantType Type();

	template<typename TYPE>
	static size_t TypeSize() { return TypeSize(Type<TYPE>()); }
	static size_t TypeSize(MEVariantType eType);

	template<typename TYPE>
	bool IsType() const { return m_eType == Type<TYPE>(); }
	bool IsValid() const { return GetType() != MEVariantType::ENone; }

	size_t GetOffset() const;
	size_t GetSize() const;
	MByte* GetData() const;
	MEVariantType GetType() const;

	template<typename TYPE>
	TYPE& GetValue();

	template<typename TYPE>
	const TYPE& GetValue() const;

	template<typename TYPE>
	void SetValue(const TYPE& value);

public:

	const MVariant& operator=(const MVariant& other);
	void operator=(MVariant&& other);

	void ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset);

public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

private:

	size_t m_nOffset = 0;
	size_t m_nSize = 0;
	MEVariantType m_eType = MEVariantType::ENone;

	std::shared_ptr<MVariantMemory> m_pMemory = nullptr;
	std::shared_ptr<MVariantStruct> m_pStruct = nullptr;
	std::shared_ptr<MVariantArray> m_pArray = nullptr;
};

class MORTY_API MVariantStruct
{
public:
	friend class MVariantStructBuilder;
	MVariantStruct();

private:

	template<typename TYPE>
	void AppendVariant(const MString& strName, const TYPE& value);

	template<typename TYPE>
	void AppendContainer(const MString& strName, const TYPE& value);

public:

	bool HasVariant(const MString& strName);

	template<typename TYPE>
	TYPE& GetVariant(const MString& strName);

	template<typename TYPE>
	void SetVariant(const MString& strName, const TYPE& value);

	MVariant& FindVariant(const MString& strName);

	MByte* Data() const { return m_pMemory->Data() + m_nOffset; }
	size_t Size() const { return m_nSize; }
	size_t Offset() const { return m_nOffset; }

	void ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset);

	static std::shared_ptr<MVariantStruct> Clone(const std::shared_ptr<MVariantStruct>& source, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

	const std::map<MString, MVariant>& GetMember() { return m_tMember; }

public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

private:

	std::shared_ptr<MVariantMemory> m_pMemory = nullptr;
	size_t m_nOffset = 0;
	size_t m_nSize = 0;
	std::map<MString, MVariant> m_tMember;
	bool m_bLocked = false;
};

class MORTY_API MVariantStructBuilder final
{
public:
	explicit MVariantStructBuilder(MVariantStruct& sut) : m_struct(sut) {}
	~MVariantStructBuilder()
	{
		MORTY_ASSERT(m_struct.m_bLocked);
	}

	template<typename TYPE>
	void AppendVariant(const MString& strName, const TYPE& value)
	{
		MORTY_ASSERT(!m_struct.m_bLocked);
		m_struct.AppendVariant(strName, value);
	}

	void Finish() const
	{
		m_struct.m_bLocked = true;
	}

private:
	MVariantStruct& m_struct;
};

typedef MVariantStruct MStruct;

class MORTY_API MVariantArray
{
public:
	friend class MVariantArrayBuilder;
	MVariantArray();

private:

	template<typename TYPE>
	void AppendVariant(const TYPE& value);

	template<typename TYPE>
	void AppendContainer(const TYPE& value);

public:

	template<typename TYPE>
	const TYPE& GetVariant(const size_t& nIdx) const;

	template<typename TYPE>
	void SetVariant(const size_t& nIdx, const TYPE& value);

	MVariant& operator[](const size_t& nIdx);

	MByte* Data() const { return m_pMemory->Data() + m_nOffset; }
	size_t Size() const { return m_nSize; }
	size_t Offset() const { return m_nOffset; }
	size_t MemberNum() const { return m_tMember.size(); }

	void ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset);

	static std::shared_ptr<MVariantArray> Clone(const std::shared_ptr<MVariantArray>& source, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

	const std::vector<MVariant>& GetMember() const { return m_tMember; }

public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

private:

	std::shared_ptr<MVariantMemory> m_pMemory = nullptr;
	size_t m_nOffset = 0;
	size_t m_nSize = 0;
	std::vector<MVariant> m_tMember;
	bool m_bLocked = false;
};

class MORTY_API MVariantArrayBuilder final
{
public:
	explicit MVariantArrayBuilder(MVariantArray& sut) : m_array(sut) {}

	~MVariantArrayBuilder()
	{
		MORTY_ASSERT(m_array.m_bLocked);
	}

	template<typename TYPE>
	void AppendVariant(const TYPE& value)
	{
		MORTY_ASSERT(!m_array.m_bLocked);
		m_array.AppendVariant(value);
	}

	void Finish() const
	{
		m_array.m_bLocked = true;
	}

private:
	MVariantArray& m_array;
};












template<typename TYPE>
inline MVariant::MVariant(const TYPE& value)
{
	MORTY_ASSERT(MVariant::Type<TYPE>() != MEVariantType::ENone);

	m_pMemory = std::make_shared<MVariantMemory>();
	m_nSize = MVariant::TypeSize<TYPE>();
	m_nOffset = m_pMemory->AllocMemory(m_nSize);
	m_eType = MVariant::Type<TYPE>();
}

template<>
inline MVariant::MVariant(const MVariantStruct& value)
{
	m_pStruct = std::make_shared<MVariantStruct>(value);
	m_nOffset = value.Offset();
	m_nSize = value.Size();
	m_eType = MEVariantType::EStruct;
}

template<>
inline MVariant::MVariant(const MVariantArray& value)
{
	m_pArray = std::make_shared<MVariantArray>(value);
	m_nOffset = value.Offset();
	m_nSize = value.Size();
	m_eType = MEVariantType::EArray;
}

template<typename TYPE>
inline TYPE& MVariant::GetValue()
{
	MORTY_ASSERT(m_pMemory);
	MORTY_ASSERT(GetType() == MVariant::Type<TYPE>());
	TYPE& value = *reinterpret_cast<TYPE*>(m_pMemory->Data() + m_nOffset);
	return value;
}

template<typename TYPE>
inline const TYPE& MVariant::GetValue() const
{
	MORTY_ASSERT(m_pMemory);
	MORTY_ASSERT(GetType() == MVariant::Type<TYPE>());
	const TYPE& value = *reinterpret_cast<TYPE*>(m_pMemory->Data() + m_nOffset);
	return value;
}

template<>
inline MVariantStruct& MVariant::GetValue<MVariantStruct>()
{
	MORTY_ASSERT(m_pStruct);
	return *m_pStruct;
}

template<>
inline const MVariantStruct& MVariant::GetValue<MVariantStruct>() const
{
	MORTY_ASSERT(m_pStruct);
	return *m_pStruct;
}

template<>
inline MVariantArray& MVariant::GetValue<MVariantArray>()
{
	MORTY_ASSERT(m_pArray);
	return *m_pArray;
}

template<>
inline const MVariantArray& MVariant::GetValue<MVariantArray>() const
{
	MORTY_ASSERT(m_pArray);
	return *m_pArray;
}

template<typename TYPE>
inline void MVariant::SetValue(const TYPE& value)
{
	MORTY_ASSERT(Type<TYPE>() == GetType());
	MORTY_ASSERT(Type<TYPE>() != MEVariantType::EArray);
	MORTY_ASSERT(Type<TYPE>() != MEVariantType::EStruct);
	MORTY_ASSERT(m_pMemory);

	memcpy(m_pMemory->Data() + m_nOffset, &value, m_nSize);
}

template<typename TYPE>
inline MEVariantType MVariant::Type()
{
	MORTY_ASSERT(false);
	return MEVariantType::ENone;
}

template<>
inline MEVariantType MVariant::Type<bool>()
{
	return MEVariantType::EUInt;
}

template<>
inline MEVariantType MVariant::Type<uint32_t>()
{
	return MEVariantType::EUInt;
}

template<>
inline MEVariantType MVariant::Type<int>()
{
	return MEVariantType::EInt;
}

template<>
inline MEVariantType MVariant::Type<float>()
{
	return MEVariantType::EFloat;
}

template<>
inline MEVariantType MVariant::Type<Vector2>()
{
	return MEVariantType::EVector2;
}

template<>
inline MEVariantType MVariant::Type<Vector3>()
{
	return MEVariantType::EVector3;
}

template<>
inline MEVariantType MVariant::Type<Vector4>()
{
	return MEVariantType::EVector4;
}

template<>
inline MEVariantType MVariant::Type<Matrix3>()
{
	return MEVariantType::EMatrix3;
}

template<>
inline MEVariantType MVariant::Type<Matrix4>()
{
	return MEVariantType::EMatrix4;
}

template<>
inline MEVariantType MVariant::Type<MVariantStruct>()
{
	return MEVariantType::EStruct;
}

template<>
inline MEVariantType MVariant::Type<MVariantArray>()
{
	return MEVariantType::EArray;
}


template<typename TYPE>
void MVariantStruct::AppendVariant(const MString& strName, const TYPE& value)
{
	MORTY_ASSERT(!m_bLocked);
	const size_t nSize = MVariant::TypeSize<TYPE>();
	const size_t nOffset = m_pMemory->AllocMemory(nSize);

	m_nSize = nOffset + nSize - m_nOffset;

	MVariant& member = m_tMember[strName];
	member = MVariant(m_pMemory, nOffset, nSize, MVariant::Type<TYPE>());

	memcpy(m_pMemory->Data() + nOffset, &value, member.GetSize());
}

template<>
inline void MVariantStruct::AppendVariant<MVariant>(const MString& strName, const MVariant& value)
{
	MORTY_ASSERT(!m_bLocked);
	if(value.IsType<MVariantStruct>())
	{
		return AppendContainer(strName, value.GetValue<MVariantStruct>());
	}
	if (value.IsType<MVariantArray>())
	{
		return AppendContainer(strName, value.GetValue<MVariantArray>());
	}

	const size_t nSize = value.GetSize();
	const size_t nOffset = m_pMemory->AllocMemory(nSize);

	m_nSize = nOffset + nSize - m_nOffset;

	MVariant& member = m_tMember[strName];
	member = std::move(MVariant(m_pMemory, nOffset, nSize, value.GetType()));

	memcpy(m_pMemory->Data() + nOffset, &value, member.GetSize());
}

template<>
inline void MVariantStruct::AppendVariant<MVariantStruct>(const MString& strName, const MVariantStruct& value)
{
	MORTY_ASSERT(!m_bLocked);
	return AppendContainer(strName, value);
}

template<>
inline void MVariantStruct::AppendVariant<MVariantArray>(const MString& strName, const MVariantArray& value)
{
	MORTY_ASSERT(!m_bLocked);
	return AppendContainer(strName, value);
}

template<typename TYPE>
inline void MVariantStruct::AppendContainer(const MString& strName, const TYPE& value)
{
	MORTY_ASSERT(!m_bLocked);
	const size_t nSize = value.Size();
	const size_t nOffset = m_pMemory->AllocMemory(value.Size());

	MVariant& member = m_tMember[strName];

	TYPE innerValue = value;
	innerValue.ResetMemory(m_pMemory, nOffset);

	member = MVariant(innerValue);

	memcpy(m_pMemory->Data() + nOffset, value.Data(), nSize);

	m_nSize = m_pMemory->Size() - m_nOffset;
}

template<typename TYPE>
inline TYPE& MVariantStruct::GetVariant(const MString& strName)
{
	MORTY_ASSERT(m_bLocked);
	MVariant& member = FindVariant(strName);
	return member.GetValue<TYPE>();
}


template<>
inline MVariant& MVariantStruct::GetVariant<MVariant>(const MString& strName)
{
	MORTY_ASSERT(m_bLocked);
	auto findResult = m_tMember.find(strName);
	if (findResult == m_tMember.end())
	{
		static MVariant InvalidValue;
		return InvalidValue;
	}

	MVariant& member = findResult->second;
	return member;
}

template<typename TYPE>
inline void MVariantStruct::SetVariant(const MString& strName, const TYPE& value)
{
	MORTY_ASSERT(m_bLocked);
	auto findResult = m_tMember.find(strName);
	if (findResult == m_tMember.end())
	{
		return;
	}

	MVariant& member = findResult->second;
	return member.SetValue(value);
}

template<>
inline void MVariantStruct::SetVariant<MVariant>(const MString& strName, const MVariant& value)
{
	MORTY_ASSERT(m_bLocked);
	MORTY_ASSERT(value.GetType() != MEVariantType::ENone);
	MORTY_ASSERT(value.GetType() != MEVariantType::EArray);
	MORTY_ASSERT(value.GetType() != MEVariantType::EStruct);

	auto findResult = m_tMember.find(strName);
	if (findResult == m_tMember.end())
	{
		return;
	}

	MVariant& member = findResult->second;
	return member.SetValue(value);
}

template<typename TYPE>
void MVariantArray::AppendVariant(const TYPE& value)
{
	MORTY_ASSERT(!m_bLocked);
	const size_t nSize = MVariant::TypeSize<TYPE>();
	const size_t nOffset = m_pMemory->AllocMemory(nSize);

	MVariant member = MVariant(m_pMemory, nOffset, nSize, MVariant::Type<TYPE>());
	memcpy(m_pMemory->Data() + nOffset, &value, member.GetSize());

	m_tMember.push_back(member);
	m_pMemory->ByteAlignment();

	m_nSize = m_pMemory->Size() - m_nOffset;
}

template<>
inline void MVariantArray::AppendVariant<MVariant>(const MVariant& value)
{
	MORTY_ASSERT(!m_bLocked);
	if (value.IsType<MVariantStruct>())
	{
		return AppendContainer<MVariantStruct>(value.GetValue<MVariantStruct>());
	}
	if (value.IsType<MVariantArray>())
	{
		return AppendContainer<MVariantArray>(value.GetValue<MVariantArray>());
	}

	const size_t nSize = value.GetSize();
	const size_t nOffset = m_pMemory->AllocMemory(nSize);

	MVariant member = MVariant(m_pMemory, nOffset, nSize, value.GetType());
	memcpy(m_pMemory->Data() + nOffset, &value, member.GetSize());

	m_tMember.push_back(member);
	m_pMemory->ByteAlignment();

	m_nSize = m_pMemory->Size() - m_nOffset;
}

template<>
inline void MVariantArray::AppendVariant<MVariantStruct>(const MVariantStruct& value)
{
	MORTY_ASSERT(!m_bLocked);
	return AppendContainer(value);
}

template<>
inline void MVariantArray::AppendVariant<MVariantArray>(const MVariantArray& value)
{
	MORTY_ASSERT(!m_bLocked);
	return AppendContainer(value);
}

template<typename TYPE>
inline void MVariantArray::AppendContainer(const TYPE& value)
{
	MORTY_ASSERT(!m_bLocked);
	const size_t nSize = value.Size();
	const size_t nOffset = m_pMemory->AllocMemory(value.Size());

	TYPE innerValue = value;
	innerValue.ResetMemory(m_pMemory, nOffset);
	MVariant member = MVariant(innerValue);

	memcpy(m_pMemory->Data() + nOffset, value.Data(), nSize);

	m_tMember.push_back(member);
	m_pMemory->ByteAlignment();

	m_nSize = m_pMemory->Size() - m_nOffset;
}

template<typename TYPE>
inline const TYPE& MVariantArray::GetVariant(const size_t& nIdx) const
{
	MORTY_ASSERT(m_bLocked);
	if (nIdx >= m_tMember.size())
	{
		MORTY_ASSERT(nIdx < m_tMember.size());
		static TYPE InvalidValue;
		return InvalidValue;
	}

	const MVariant& member = m_tMember[nIdx];
	return member.GetValue<TYPE>();
}

template<typename TYPE>
inline void MVariantArray::SetVariant(const size_t& nIdx, const TYPE& value)
{
	MORTY_ASSERT(m_bLocked);
	if (nIdx >= m_tMember.size())
	{
		MORTY_ASSERT(nIdx < m_tMember.size());
		return;
	}

	MVariant& member = m_tMember[nIdx];
	return member.SetValue(value);
}
