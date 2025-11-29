#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MString.h"
#include <vector>

namespace morty
{

// Serialization traits for converting types to/from strings
template<typename T> struct IniSerializer
{
    static MString ToString(const T& value)
    {
        MORTY_UNUSED(value);
        MORTY_ASSERT(false && "No serializer defined for this type");
        return "";
    }

    static T FromString(const MString& str)
    {
        MORTY_UNUSED(str);
        MORTY_ASSERT(false && "No deserializer defined for this type");
        return {};
    }
};

// Specializations for basic types
template<> struct IniSerializer<MString>
{
    static MString ToString(const MString& value) { return value; }
    static MString FromString(const MString& str) { return str; }
};

template<> struct IniSerializer<int>
{
    static MString ToString(const int& value) { return std::to_string(value); }
    static int     FromString(const MString& str)
    {
        if (str.empty()) return 0;
        return std::stoi(str);
    }
};

template<> struct IniSerializer<bool>
{
    static MString ToString(const bool& value) { return value ? "1" : "0"; }
    static bool    FromString(const MString& str) { return IniSerializer<int>::FromString(str) != 0; }
};

template<> struct IniSerializer<Vector2i>
{
    static MString  ToString(const Vector2i& value) { return std::to_string(value.x) + "," + std::to_string(value.y); }
    static Vector2i FromString(const MString& str)
    {
        if (str.empty()) return {};
        auto values = MStringUtil::Slip(str, ",");
        MORTY_ASSERT(values.size() == 2);
        return {std::stoi(values[0]), std::stoi(values[1])};
    }
};

class IniConfig
{

public:
    explicit IniConfig();

    template<typename T> T              GetValue(const char* section, const char* name);
    template<typename T> void           SetValue(const char* section, const char* name, const T& value);

    template<typename T> void           SetArray(const char* section, const char* name, const T* array, size_t size);
    template<typename T> void           SetArray(const char* section, const char* name, const std::vector<T>& array);
    template<typename T> std::vector<T> GetArray(const char* section, const char* name);

    void                                LoadFromFile(const MString& filePath);

    void                                Save(const MString& filePath);

private:
    static int Parse(void* user, const char* section, const char* name, const char* value);

    std::map<MString, std::map<MString, MString>> m_config;
};

// GetValue and SetValue now use IniSerializer
template<typename T> inline T IniConfig::GetValue(const char* section, const char* name)
{
    auto sectionResult = m_config.find(section);
    if (sectionResult == m_config.end()) return {};
    auto nameResult = sectionResult->second.find(name);
    if (nameResult == sectionResult->second.end()) return {};

    return IniSerializer<T>::FromString(nameResult->second);
}

template<typename T> inline void IniConfig::SetValue(const char* section, const char* name, const T& value)
{
    m_config[section][name] = IniSerializer<T>::ToString(value);
}

// Array serialization using the same IniSerializer infrastructure
template<typename T> inline void IniConfig::SetArray(const char* section, const char* name, const T* array, size_t size)
{
    // First, save the array size
    SetValue<int>(section, (MString(name) + "_Count").c_str(), static_cast<int>(size));

    // Then save each element using IniSerializer
    for (size_t i = 0; i < size; ++i)
    {
        MString key = MString(name) + "_" + std::to_string(i);
        SetValue<T>(section, key.c_str(), array[i]);
    }
}

template<typename T> inline void IniConfig::SetArray(const char* section, const char* name, const std::vector<T>& array)
{
    SetArray<T>(section, name, array.data(), array.size());
}

template<typename T> inline std::vector<T> IniConfig::GetArray(const char* section, const char* name)
{
    // First, get the array size
    int count = GetValue<int>(section, (MString(name) + "_Count").c_str());

    std::vector<T> result;
    result.reserve(count);

    // Then load each element using IniSerializer
    for (int i = 0; i < count; ++i)
    {
        MString key = MString(name) + "_" + std::to_string(i);
        result.push_back(GetValue<T>(section, key.c_str()));
    }

    return result;
}

}// namespace morty