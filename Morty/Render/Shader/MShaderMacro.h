/**
 * @File         MShader
 * 
 * @Created      2020-5-8 22:15:46
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MStringId.h"
#include "Variant/MVariant.h"

namespace morty
{

class MORTY_API MShaderMacro
{
public:
    void                      SetMacro(const MStringId& strKey, const MString& strValue);

    void                      AddUnionMacro(const MStringId& strKey, const MString& strValue = "");

    void                      RemoveMacro(const MStringId& strKey);

    bool                      HasMacro(const MStringId& strKey) const;

    MString                   GetMacro(const MStringId& strKey) const;

    bool                      Compare(const MShaderMacro& macro);

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                      Deserialize(const void* pBufferPointer);

protected:
    friend class MMaterial;

    void SetMacro(const MStringId& strKey, const MString& strValue, std::unordered_map<MStringId, MString>& table);

public:
    std::unordered_map<MStringId, MString>        m_macroParams;
    std::unordered_map<MStringId, MString>        m_mortyMacroParams;

    static std::unordered_map<MStringId, MString> s_vGlobalMacroParams;
};

}// namespace morty