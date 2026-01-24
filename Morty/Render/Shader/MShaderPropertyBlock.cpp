#include "MShaderPropertyBlock.h"

using namespace morty;

bool MShaderPropertyBlock::AddProperty(const MShaderParamAttribute& property)
{
    if (m_properties.find(property.name) != m_properties.end())
    {
        if (m_properties[property.name].type != property.type) { return false; }
        return true;
    }

    m_properties[property.name] = property;
    return true;
}

bool MShaderPropertyBlock::AddResource(const MShaderParamResource& resource)
{
    if (m_resources.find(resource.name) != m_resources.end())
    {
        if (m_resources[resource.name].type != resource.type) { return false; }
        return true;
    }

    m_resources[resource.name] = resource;
    return true;
}

const std::unordered_map<MStringId, MShaderParamAttribute>& MShaderPropertyBlock::GetProperties() const { return m_properties; }

const std::unordered_map<MStringId, MShaderParamResource>&  MShaderPropertyBlock::GetResources() const { return m_resources; }

const std::array<MShaderInstancingAttribute, 2>&            MShaderPropertyBlock::GetInstancingProperties() const { return m_instancingProperties; }

MStringId                                                   MShaderPropertyBlock::GetPropertyDisplayName(const MStringId& name) const
{
    auto result = m_properties.find(name);
    if (result == m_properties.end()) { return name; }

    return result->second.displayName;
}

MStringId MShaderPropertyBlock::GetResourceDisplayName(const MStringId& name) const
{
    auto result = m_resources.find(name);
    if (result == m_resources.end()) { return name; }

    return result->second.displayName;
}

MStringId MShaderPropertyBlock::GetInstancingName(MInstanceDataType type) const
{
    auto index = static_cast<size_t>(type);
    if (index < m_instancingProperties.size()) { return m_instancingProperties[index].name; }

    return MStringId::Empty;
}

bool MShaderPropertyBlock::SetInstancingProperty(MInstanceDataType type, const MShaderInstancingAttribute& property)
{
    auto index = static_cast<size_t>(type);
    if (index < m_instancingProperties.size())
    {
        m_instancingProperties[index] = property;
        return true;
    }

    return false;
}

void MShaderPropertyBlock::Merge(const MShaderPropertyBlock& other)
{
    for (const auto& [name, property]: other.m_properties) { AddProperty(property); }
    for (const auto& [name, resource]: other.m_resources) { AddResource(resource); }

    m_instancingProperties = other.m_instancingProperties;
}

void MShaderPropertyBlock::Clear()
{
    m_properties.clear();
    m_resources.clear();
}