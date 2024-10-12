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
#include "MVariantMemory.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Utility/MString.h"
#include "Utility/MStringId.h"

namespace morty
{

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

    template<typename TYPE> explicit MVariant(const TYPE& value);

    static MVariant Clone(const MVariant& value, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

    MVariant(const MVariant& value) = default;

    MVariant(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset, size_t nSize, MEVariantType eType);

    template<typename TYPE> static MEVariantType Type();

    template<typename TYPE> static size_t        TypeSize() { return TypeSize(Type<TYPE>()); }

    static size_t                                TypeSize(MEVariantType eType);

    template<typename TYPE> bool                 IsType() const { return m_type == Type<TYPE>(); }

    bool                                         IsValid() const { return GetType() != MEVariantType::ENone; }

    size_t                                       GetOffset() const;

    size_t                                       GetSize() const;

    MByte*                                       GetData() const;

    MEVariantType                                GetType() const;

    template<typename TYPE> TYPE&                GetValue();

    template<typename TYPE> const TYPE&          GetValue() const;

    template<typename TYPE> void                 SetValue(const TYPE& value);

public:
    const MVariant& operator=(const MVariant& other);

    void            operator=(MVariant&& other);

    void            ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset);

public:
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

private:
    size_t                          m_offset = 0;
    size_t                          m_size   = 0;
    MEVariantType                   m_type   = MEVariantType::ENone;

    std::shared_ptr<MVariantMemory> m_memory = nullptr;
    std::shared_ptr<MVariantStruct> m_struct = nullptr;
    std::shared_ptr<MVariantArray>  m_array  = nullptr;
};

class MORTY_API MVariantStruct
{
public:
    friend class MVariantStructBuilder;

    MVariantStruct();

private:
    template<typename TYPE> void AppendVariant(const MStringId& strName, const TYPE& value);

    template<typename TYPE> void AppendContainer(const MStringId& strName, const TYPE& value);

public:
    bool                          HasVariant(const MStringId& strName);

    template<typename TYPE> TYPE& GetVariant(const MStringId& strName);

    template<typename TYPE> void  SetVariant(const MStringId& strName, const TYPE& value);

    MVariant&                     FindVariant(const MStringId& strName);

    MByte*                        Data() const { return m_memory->Data() + m_offset; }

    size_t                        Size() const { return m_size; }

    size_t                        Offset() const { return m_offset; }

    void                          ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset);

    static std::shared_ptr<MVariantStruct>
                                                   Clone(const std::shared_ptr<MVariantStruct>& source,
                                                         std::shared_ptr<MVariantMemory>        pMemory = nullptr,
                                                         size_t                                 nOffset = 0);

    const std::unordered_map<MStringId, MVariant>& GetMember() { return m_member; }

public:
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

private:
    std::shared_ptr<MVariantMemory>         m_memory = nullptr;
    size_t                                  m_offset = 0;
    size_t                                  m_size   = 0;
    std::unordered_map<MStringId, MVariant> m_member;
    bool                                    m_locked = false;
};

class MORTY_API MVariantStructBuilder final
{
public:
    explicit MVariantStructBuilder(MVariantStruct& sut)
        : m_struct(sut)
    {}

    ~MVariantStructBuilder() { MORTY_ASSERT(m_struct.m_locked); }

    template<typename TYPE> MVariantStructBuilder& AppendVariant(const MStringId& strName, const TYPE& value)
    {
        MORTY_ASSERT(!m_struct.m_locked);
        m_struct.AppendVariant(strName, value);
        return *this;
    }

    void Finish() const { m_struct.m_locked = true; }

private:
    MVariantStruct& m_struct;
};

typedef MVariantStruct MStruct;

class MORTY_API        MVariantArray
{
public:
    friend class MVariantArrayBuilder;

    MVariantArray();

private:
    template<typename TYPE> void AppendVariant(const TYPE& value);

    template<typename TYPE> void AppendContainer(const TYPE& value);

public:
    template<typename TYPE> const TYPE& GetVariant(const size_t& nIdx) const;

    template<typename TYPE> void        SetVariant(const size_t& nIdx, const TYPE& value);

    MVariant&                           operator[](const size_t& nIdx);

    MByte*                              Data() const { return m_memory->Data() + m_offset; }

    size_t                              Size() const { return m_size; }

    size_t                              Offset() const { return m_offset; }

    size_t                              MemberNum() const { return m_member.size(); }

    void                                ResetMemory(const std::shared_ptr<MVariantMemory>& pMemory, size_t nOffset);

    static std::shared_ptr<MVariantArray>
                                 Clone(const std::shared_ptr<MVariantArray>& source,
                                       std::shared_ptr<MVariantMemory>       pMemory = nullptr,
                                       size_t                                nOffset = 0);

    const std::vector<MVariant>& GetMember() const { return m_member; }

public:
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void Deserialize(const void* pBufferPointer, std::shared_ptr<MVariantMemory> pMemory = nullptr, size_t nOffset = 0);

private:
    std::shared_ptr<MVariantMemory> m_memory = nullptr;
    size_t                          m_offset = 0;
    size_t                          m_size   = 0;
    std::vector<MVariant>           m_member;
    bool                            m_locked = false;
};

class MORTY_API MVariantArrayBuilder final
{
public:
    explicit MVariantArrayBuilder(MVariantArray& sut)
        : m_array(sut)
    {}

    ~MVariantArrayBuilder() { MORTY_ASSERT(m_array.m_locked); }

    template<typename TYPE> void AppendVariant(const TYPE& value)
    {
        MORTY_ASSERT(!m_array.m_locked);
        m_array.AppendVariant(value);
    }

    void Finish() const { m_array.m_locked = true; }

private:
    MVariantArray& m_array;
};


template<typename TYPE> inline MVariant::MVariant(const TYPE& value)
{
    MORTY_ASSERT(MVariant::Type<TYPE>() != MEVariantType::ENone);

    m_memory = std::make_shared<MVariantMemory>();
    m_size   = MVariant::TypeSize<TYPE>();
    m_offset = m_memory->AllocMemory(m_size);
    m_type   = MVariant::Type<TYPE>();

    memcpy(m_memory->Data() + m_offset, &value, m_size);
}

template<> inline MVariant::MVariant(const MVariantStruct& value)
{
    m_struct = std::make_shared<MVariantStruct>(value);
    m_offset = value.Offset();
    m_size   = value.Size();
    m_type   = MEVariantType::EStruct;
}

template<> inline MVariant::MVariant(const MVariantArray& value)
{
    m_array  = std::make_shared<MVariantArray>(value);
    m_offset = value.Offset();
    m_size   = value.Size();
    m_type   = MEVariantType::EArray;
}

template<typename TYPE> inline TYPE& MVariant::GetValue()
{
    MORTY_ASSERT(m_memory);
    MORTY_ASSERT(GetType() == MVariant::Type<TYPE>());
    TYPE& value = *reinterpret_cast<TYPE*>(m_memory->Data() + m_offset);
    return value;
}

template<typename TYPE> inline const TYPE& MVariant::GetValue() const
{
    MORTY_ASSERT(m_memory);
    MORTY_ASSERT(GetType() == MVariant::Type<TYPE>());
    const TYPE& value = *reinterpret_cast<TYPE*>(m_memory->Data() + m_offset);
    return value;
}

template<> inline MVariantStruct& MVariant::GetValue<MVariantStruct>()
{
    MORTY_ASSERT(m_struct);
    return *m_struct;
}

template<> inline const MVariantStruct& MVariant::GetValue<MVariantStruct>() const
{
    MORTY_ASSERT(m_struct);
    return *m_struct;
}

template<> inline MVariantArray& MVariant::GetValue<MVariantArray>()
{
    MORTY_ASSERT(m_array);
    return *m_array;
}

template<> inline const MVariantArray& MVariant::GetValue<MVariantArray>() const
{
    MORTY_ASSERT(m_array);
    return *m_array;
}

template<typename TYPE> inline void MVariant::SetValue(const TYPE& value)
{
    MORTY_ASSERT(Type<TYPE>() == GetType());
    MORTY_ASSERT(Type<TYPE>() != MEVariantType::EArray);
    MORTY_ASSERT(Type<TYPE>() != MEVariantType::EStruct);
    MORTY_ASSERT(m_memory);

    memcpy(m_memory->Data() + m_offset, &value, m_size);
}

template<typename TYPE> inline MEVariantType MVariant::Type()
{
    MORTY_ASSERT(false);
    return MEVariantType::ENone;
}

template<> inline MEVariantType MVariant::Type<bool>() { return MEVariantType::EUInt; }

template<> inline MEVariantType MVariant::Type<uint32_t>() { return MEVariantType::EUInt; }

template<> inline MEVariantType MVariant::Type<int>() { return MEVariantType::EInt; }

template<> inline MEVariantType MVariant::Type<float>() { return MEVariantType::EFloat; }

template<> inline MEVariantType MVariant::Type<Vector2>() { return MEVariantType::EVector2; }

template<> inline MEVariantType MVariant::Type<Vector3>() { return MEVariantType::EVector3; }

template<> inline MEVariantType MVariant::Type<Vector4>() { return MEVariantType::EVector4; }

template<> inline MEVariantType MVariant::Type<Matrix3>() { return MEVariantType::EMatrix3; }

template<> inline MEVariantType MVariant::Type<Matrix4>() { return MEVariantType::EMatrix4; }

template<> inline MEVariantType MVariant::Type<MVariantStruct>() { return MEVariantType::EStruct; }

template<> inline MEVariantType MVariant::Type<MVariantArray>() { return MEVariantType::EArray; }


template<typename TYPE> void    MVariantStruct::AppendVariant(const MStringId& strName, const TYPE& value)
{
    MORTY_ASSERT(!m_locked);
    const size_t nSize   = MVariant::TypeSize<TYPE>();
    const size_t nOffset = m_memory->AllocMemory(nSize);

    m_size = nOffset + nSize - m_offset;

    MVariant& member = m_member[strName];
    member           = MVariant(m_memory, nOffset, nSize, MVariant::Type<TYPE>());

    memcpy(m_memory->Data() + nOffset, &value, member.GetSize());
}

template<> inline void MVariantStruct::AppendVariant<MVariant>(const MStringId& strName, const MVariant& value)
{
    MORTY_ASSERT(!m_locked);
    if (value.IsType<MVariantStruct>()) { return AppendContainer(strName, value.GetValue<MVariantStruct>()); }
    if (value.IsType<MVariantArray>()) { return AppendContainer(strName, value.GetValue<MVariantArray>()); }

    const size_t nSize   = value.GetSize();
    const size_t nOffset = m_memory->AllocMemory(nSize);

    m_size = nOffset + nSize - m_offset;

    MVariant& member = m_member[strName];
    member           = MVariant(m_memory, nOffset, nSize, value.GetType());

    memcpy(m_memory->Data() + nOffset, &value, member.GetSize());
}

template<>
inline void MVariantStruct::AppendVariant<MVariantStruct>(const MStringId& strName, const MVariantStruct& value)
{
    MORTY_ASSERT(!m_locked);
    return AppendContainer(strName, value);
}

template<>
inline void MVariantStruct::AppendVariant<MVariantArray>(const MStringId& strName, const MVariantArray& value)
{
    MORTY_ASSERT(!m_locked);
    return AppendContainer(strName, value);
}

template<typename TYPE> inline void MVariantStruct::AppendContainer(const MStringId& strName, const TYPE& value)
{
    MORTY_ASSERT(!m_locked);
    const size_t nSize   = value.Size();
    const size_t nOffset = m_memory->AllocMemory(value.Size());

    MVariant&    member = m_member[strName];

    TYPE         innerValue = value;
    innerValue.ResetMemory(m_memory, nOffset);

    member = MVariant(innerValue);

    memcpy(m_memory->Data() + nOffset, value.Data(), nSize);

    m_size = m_memory->Size() - m_offset;
}

template<typename TYPE> inline TYPE& MVariantStruct::GetVariant(const MStringId& strName)
{
    MORTY_ASSERT(m_locked);
    MVariant& member = FindVariant(strName);
    return member.GetValue<TYPE>();
}


template<> inline MVariant& MVariantStruct::GetVariant<MVariant>(const MStringId& strName)
{
    MORTY_ASSERT(m_locked);
    auto findResult = m_member.find(strName);
    if (findResult == m_member.end())
    {
        static MVariant InvalidValue;
        return InvalidValue;
    }

    MVariant& member = findResult->second;
    return member;
}

template<typename TYPE> inline void MVariantStruct::SetVariant(const MStringId& strName, const TYPE& value)
{
    MORTY_ASSERT(m_locked);
    auto findResult = m_member.find(strName);
    if (findResult == m_member.end()) { return; }

    MVariant& member = findResult->second;
    return member.SetValue(value);
}

template<> inline void MVariantStruct::SetVariant<MVariant>(const MStringId& strName, const MVariant& value)
{
    MORTY_ASSERT(m_locked);
    MORTY_ASSERT(value.GetType() != MEVariantType::ENone);
    MORTY_ASSERT(value.GetType() != MEVariantType::EArray);
    MORTY_ASSERT(value.GetType() != MEVariantType::EStruct);

    auto findResult = m_member.find(strName);
    if (findResult == m_member.end()) { return; }

    MVariant& member = findResult->second;
    return member.SetValue(value);
}

template<typename TYPE> void MVariantArray::AppendVariant(const TYPE& value)
{
    MORTY_ASSERT(!m_locked);
    const size_t nSize   = MVariant::TypeSize<TYPE>();
    const size_t nOffset = m_memory->AllocMemory(nSize);

    MVariant     member = MVariant(m_memory, nOffset, nSize, MVariant::Type<TYPE>());
    memcpy(m_memory->Data() + nOffset, &value, member.GetSize());

    m_member.push_back(member);
    m_memory->ByteAlignment();

    m_size = m_memory->Size() - m_offset;
}

template<> inline void MVariantArray::AppendVariant<MVariant>(const MVariant& value)
{
    MORTY_ASSERT(!m_locked);
    if (value.IsType<MVariantStruct>()) { return AppendContainer<MVariantStruct>(value.GetValue<MVariantStruct>()); }
    if (value.IsType<MVariantArray>()) { return AppendContainer<MVariantArray>(value.GetValue<MVariantArray>()); }

    const size_t nSize   = value.GetSize();
    const size_t nOffset = m_memory->AllocMemory(nSize);

    MVariant     member = MVariant(m_memory, nOffset, nSize, value.GetType());
    memcpy(m_memory->Data() + nOffset, &value, member.GetSize());

    m_member.push_back(member);
    m_memory->ByteAlignment();

    m_size = m_memory->Size() - m_offset;
}

template<> inline void MVariantArray::AppendVariant<MVariantStruct>(const MVariantStruct& value)
{
    MORTY_ASSERT(!m_locked);
    return AppendContainer(value);
}

template<> inline void MVariantArray::AppendVariant<MVariantArray>(const MVariantArray& value)
{
    MORTY_ASSERT(!m_locked);
    return AppendContainer(value);
}

template<typename TYPE> inline void MVariantArray::AppendContainer(const TYPE& value)
{
    MORTY_ASSERT(!m_locked);
    const size_t nSize   = value.Size();
    const size_t nOffset = m_memory->AllocMemory(value.Size());

    TYPE         innerValue = value;
    innerValue.ResetMemory(m_memory, nOffset);
    MVariant member = MVariant(innerValue);

    memcpy(m_memory->Data() + nOffset, value.Data(), nSize);

    m_member.push_back(member);
    m_memory->ByteAlignment();

    m_size = m_memory->Size() - m_offset;
}

template<typename TYPE> inline const TYPE& MVariantArray::GetVariant(const size_t& nIdx) const
{
    MORTY_ASSERT(m_locked);
    if (nIdx >= m_member.size())
    {
        MORTY_ASSERT(nIdx < m_member.size());
        static TYPE InvalidValue;
        return InvalidValue;
    }

    const MVariant& member = m_member[nIdx];
    return member.GetValue<TYPE>();
}

template<typename TYPE> inline void MVariantArray::SetVariant(const size_t& nIdx, const TYPE& value)
{
    MORTY_ASSERT(m_locked);
    if (nIdx >= m_member.size())
    {
        MORTY_ASSERT(nIdx < m_member.size());
        return;
    }

    MVariant& member = m_member[nIdx];
    return member.SetValue(value);
}

}// namespace morty