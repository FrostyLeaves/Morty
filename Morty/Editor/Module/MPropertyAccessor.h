/**
 * @File         MPropertyAccessor
 *
 * @Created      2026-01-22
 *
 * @Author       DoubleYe
 **/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#include <vector>

namespace morty
{

class MEngine;
class MScene;
class MEntity;

// Parsed command path
struct MPropertyPath
{
    std::vector<MString> nodePath;// e.g., ["root", "node_1", "child_2"]
    MString              componentName;// e.g., "MSceneComponent"
    MString              propertyName; // e.g., "Visible"
    bool                 valid = false;
};

// Result of property operation
struct MPropertyResult
{
    bool    success = false;
    MString value;
    MString errorMessage;
};

class MORTY_API MPropertyAccessor
{
public:
    // Parse path string: "root/node_1/child_2:MSceneComponent:Visible"
    static MPropertyPath ParsePath(const MString& pathStr);

    // Find entity by path in scene hierarchy
    static MEntity* FindEntityByPath(MScene* scene, const std::vector<MString>& path);

    // Get property value as string
    static MPropertyResult GetProperty(MScene* scene, const MPropertyPath& path);

    // Set property value from string
    static MPropertyResult SetProperty(MScene* scene, const MPropertyPath& path, const MString& value);
};

}// namespace morty
