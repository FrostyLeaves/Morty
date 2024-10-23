#include "Type/MType.h"

using namespace morty;

const MType* MTypeClass::GetClassType()
{
    static MStringId   basic("MTypeClass");
    static const MType type(basic, nullptr);
    return &type;
}

MTypeClass* MTypeClass::New(const MStringId& strTypeName)
{
    auto findResult = GetNameTable().find(strTypeName);
    if (findResult != GetNameTable().end()) return findResult->second.m_funcNew();

    return nullptr;
}

MTypeClass* MTypeClass::New(const MType* type)
{
    auto findResult = GetTypeTable().find(type);
    if (findResult != GetTypeTable().end()) return findResult->second.m_funcNew();

    return nullptr;
}

const MType* MTypeClass::GetType(const MStringId& strTypeName)
{
    auto findResult = GetNameTable().find(strTypeName);
    if (findResult != GetNameTable().end()) return findResult->second.m_type;

    return nullptr;
}

std::unordered_map<MStringId, MDynamicTypeInfo>& MTypeClass::GetNameTable()
{
    static std::unordered_map<MStringId, MDynamicTypeInfo> m;
    return m;
}

std::unordered_map<const MType*, MDynamicTypeInfo>& MTypeClass::GetTypeTable()
{
    static std::unordered_map<const MType*, MDynamicTypeInfo> m;
    return m;
}
