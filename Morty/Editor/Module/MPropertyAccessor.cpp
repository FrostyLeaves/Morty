/**
 * @File         MPropertyAccessor
 *
 * @Created      2026-01-22
 *
 * @Author       DoubleYe
 **/

#include "Module/MPropertyAccessor.h"

#include "Reflection/MComponentAccessor.gen"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Component/MSceneComponent.h"
#include "Type/MType.h"

#include <sstream>

namespace morty
{

MPropertyPath MPropertyAccessor::ParsePath(const MString& pathStr)
{
    MPropertyPath result;

    // Split by ':'
    std::vector<MString> colonParts;
    std::stringstream    ss(pathStr);
    MString              part;
    while (std::getline(ss, part, ':')) { colonParts.push_back(part); }

    if (colonParts.size() != 3)
    {
        result.valid = false;
        return result;
    }

    // Parse node path (split by '/')
    std::stringstream pathSs(colonParts[0]);
    MString           nodePart;
    while (std::getline(pathSs, nodePart, '/'))
    {
        if (!nodePart.empty()) { result.nodePath.push_back(nodePart); }
    }

    result.componentName = colonParts[1];
    result.propertyName  = colonParts[2];
    result.valid = !result.nodePath.empty() && !result.componentName.empty() && !result.propertyName.empty();

    return result;
}

MEntity* MPropertyAccessor::FindEntityByPath(MScene* scene, const std::vector<MString>& path)
{
    if (!scene || path.empty()) { return nullptr; }

    // Get all entities to find root
    auto entities = scene->GetAllEntity();

    // Find entity matching first path segment (root)
    MEntity* current = nullptr;
    for (MEntity* entity: entities)
    {
        if (entity && entity->GetName() == path[0])
        {
            // Check if this is a root (no parent)
            auto* sceneComp = entity->GetComponent<MSceneComponent>();
            if (sceneComp)
            {
                auto parentId = sceneComp->GetParentComponent();
                if (!parentId.IsValid())
                {
                    current = entity;
                    break;
                }
            }
            else
            {
                // Entity without scene component could still match
                current = entity;
                break;
            }
        }
    }

    if (!current) { return nullptr; }

    // Traverse path
    for (size_t i = 1; i < path.size(); ++i)
    {
        auto* sceneComp = current->GetComponent<MSceneComponent>();
        if (!sceneComp) { return nullptr; }

        const auto& children   = sceneComp->GetChildrenComponent();
        MEntity*    foundChild = nullptr;

        for (const auto& childId: children)
        {
            if (auto* childComp = static_cast<MSceneComponent*>(scene->GetComponent(childId)))
            {
                MEntity* childEntity = childComp->GetEntity();
                if (childEntity && childEntity->GetName() == path[i])
                {
                    foundChild = childEntity;
                    break;
                }
            }
        }

        if (!foundChild) { return nullptr; }
        current = foundChild;
    }

    return current;
}

MPropertyResult MPropertyAccessor::GetProperty(MScene* scene, const MPropertyPath& path)
{
    MPropertyResult result;

    if (!path.valid)
    {
        result.errorMessage = "Invalid path format";
        return result;
    }

    // Find entity
    MEntity* entity = FindEntityByPath(scene, path.nodePath);
    if (!entity)
    {
        result.errorMessage = "Entity not found at path";
        return result;
    }

    // Find component by type name
    const MType* compType = MTypeClass::GetType(MStringId(path.componentName));
    if (!compType)
    {
        result.errorMessage = "Unknown component type: " + path.componentName;
        return result;
    }

    MComponent* component = entity->GetComponent(compType);
    if (!component)
    {
        result.errorMessage = "Entity does not have component: " + path.componentName;
        return result;
    }

    // Look up accessor in generated registry
    const auto& registry = GetPropertyAccessorRegistry();
    auto        compIt   = registry.find(MStringId(path.componentName));
    if (compIt == registry.end())
    {
        result.errorMessage = "No property accessors for component: " + path.componentName;
        return result;
    }

    auto propIt = compIt->second.find(MStringId(path.propertyName));
    if (propIt == compIt->second.end())
    {
        result.errorMessage = "Unknown property: " + path.propertyName;
        return result;
    }

    // Call getter
    MString value = propIt->second.getter(component);
    if (value.empty() && propIt->second.propertyType != "bool")
    {
        // Empty string might be valid for bool (false), but check for errors
        result.errorMessage = "Failed to get property value";
        return result;
    }

    result.success = true;
    result.value   = value;
    return result;
}

MPropertyResult MPropertyAccessor::SetProperty(MScene* scene, const MPropertyPath& path, const MString& value)
{
    MPropertyResult result;

    if (!path.valid)
    {
        result.errorMessage = "Invalid path format";
        return result;
    }

    // Find entity
    MEntity* entity = FindEntityByPath(scene, path.nodePath);
    if (!entity)
    {
        result.errorMessage = "Entity not found at path";
        return result;
    }

    // Find component
    const MType* compType = MTypeClass::GetType(MStringId(path.componentName));
    if (!compType)
    {
        result.errorMessage = "Unknown component type: " + path.componentName;
        return result;
    }

    MComponent* component = entity->GetComponent(compType);
    if (!component)
    {
        result.errorMessage = "Entity does not have component: " + path.componentName;
        return result;
    }

    // Look up accessor
    const auto& registry = GetPropertyAccessorRegistry();
    auto        compIt   = registry.find(MStringId(path.componentName));
    if (compIt == registry.end())
    {
        result.errorMessage = "No property accessors for component: " + path.componentName;
        return result;
    }

    auto propIt = compIt->second.find(MStringId(path.propertyName));
    if (propIt == compIt->second.end())
    {
        result.errorMessage = "Unknown property: " + path.propertyName;
        return result;
    }

    // Call setter
    if (!propIt->second.setter(component, value))
    {
        result.errorMessage = "Failed to set property value";
        return result;
    }

    result.success = true;
    return result;
}

}// namespace morty
