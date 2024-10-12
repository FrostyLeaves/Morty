#include "IniConfig.h"
#include "Utility/MFileHelper.h"

#include <ini.h>

using namespace morty;


IniConfig::IniConfig() {}

int IniConfig::Parse(void* user, const char* section, const char* name, const char* value)
{
    auto config = static_cast<IniConfig*>(user);

    config->m_config[section][name] = value;

    return 1;
}

void IniConfig::Save(const MString& filePath)
{
    MString configData;
    for (const auto& pair: m_config)
    {
        configData += '[';
        configData += pair.first;
        configData += "]\n";

        for (const auto& kv: pair.second)
        {
            configData += kv.first;
            configData += '=';
            configData += kv.second;
            configData += '\n';
        }

        configData += '\n';
    }

    MFileHelper::WriteString(filePath, configData);
}

void IniConfig::LoadFromFile(const MString& filePath)
{
    m_config.clear();

    ini_parse(filePath.c_str(), IniConfig::Parse, this);
}
