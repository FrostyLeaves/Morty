#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MString.h"

namespace morty
{

class IniConfig
{

public:
    explicit IniConfig();

    template<typename T> T GetValue(const char* section, const char* name);


    template<typename T>
    void SetValue(const char* section, const char* name, const T& value);

    void LoadFromFile(const MString& filePath);

    void Save(const MString& filePath);

private:
    static int
    Parse(void* user, const char* section, const char* name, const char* value);

    std::map<MString, std::map<MString, MString>> m_config;
};

template<typename T> inline T IniConfig::GetValue(const char* section, const char* name)
{
    MORTY_ASSERT(false);

    return {};
}

template<typename T>
inline void IniConfig::SetValue(const char* section, const char* name, const T& value)
{
    MORTY_ASSERT(false);
}

template<>
inline MString IniConfig::GetValue<MString>(const char* section, const char* name)
{
    auto sectionResult = m_config.find(section);
    if (sectionResult == m_config.end()) return {};
    auto nameResult = sectionResult->second.find(name);
    if (nameResult == sectionResult->second.end()) return {};
    return nameResult->second;
}

template<> inline void
IniConfig::SetValue<MString>(const char* section, const char* name, const MString& value)
{
    m_config[section][name] = value;
}

template<> inline int IniConfig::GetValue<int>(const char* section, const char* name)
{
    auto value = GetValue<MString>(section, name);
    return std::stoi(value);
}

template<> inline void
IniConfig::SetValue<int>(const char* section, const char* name, const int& value)
{
    m_config[section][name] = std::to_string(value);
}

template<> inline bool IniConfig::GetValue<bool>(const char* section, const char* name)
{
    return GetValue<int>(section, name);
}

template<> inline void
IniConfig::SetValue<bool>(const char* section, const char* name, const bool& value)
{
    SetValue<int>(section, name, value ? 1 : 0);
}

template<>
inline Vector2i IniConfig::GetValue<Vector2i>(const char* section, const char* name)
{
    auto value = GetValue<MString>(section, name);
    if (value.empty()) { return {}; }

    auto values = MStringUtil::Slip(value, ",");
    MORTY_ASSERT(values.size() == 2);

    return {std::stoi(values[0]), std::stoi(values[1])};
}

template<> inline void IniConfig::SetValue<Vector2i>(
        const char*     section,
        const char*     name,
        const Vector2i& value
)
{
    m_config[section][name] = std::to_string(value.x) + "," + std::to_string(value.y);
}

}// namespace morty