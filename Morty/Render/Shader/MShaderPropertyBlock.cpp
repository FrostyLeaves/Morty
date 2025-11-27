#include "MShaderPropertyBlock.h"

using namespace morty;

bool MShaderPropertyBlock::AddProperty(const MStringId& name, const MShaderParamAttribute& property)
{
    if (m_properties.find(name) != m_properties.end())
    {
        if (m_properties[name].type != property.type) { return false; }
        return true;
    }

    m_properties[name] = property;
    return true;
}

bool MShaderPropertyBlock::AddResource(const MStringId& name, const MShaderParamResource& resource)
{
    if (m_resources.find(name) != m_resources.end())
    {
        if (m_resources[name].type != resource.type) { return false; }
        return true;
    }

    m_resources[name] = resource;
    return true;
}

const std::unordered_map<MStringId, MShaderParamAttribute>& MShaderPropertyBlock::GetProperties() const
{
    return m_properties;
}

const std::unordered_map<MStringId, MShaderParamResource>& MShaderPropertyBlock::GetResources() const
{
    return m_resources;
}

void MShaderPropertyBlock::Merge(const MShaderPropertyBlock& other)
{
    for (const auto& [name, property]: other.m_properties) { AddProperty(name, property); }
    for (const auto& [name, resource]: other.m_resources) { AddResource(name, resource); }
}

void MShaderPropertyBlock::Clear()
{
    m_properties.clear();
    m_resources.clear();
}