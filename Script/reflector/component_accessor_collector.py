#coding:UTF-8
import os
import re
from enum import Enum
import reflector_collector
import property_type_parser

class PropertyType(Enum):
    Variant = 1
    Enum = 2
    Resource = 3
    Struct = 4

class PropertyInfo:
    def __init__(self):
        self.name = ""
        self.type = ""
        self.getter = ""
        self.setter = ""
        self.property_type = PropertyType.Variant
        self.enum_values = {}

class ComponentInfo:
    def __init__(self):
        self.class_name = ""
        self.properties = []

# Template for the generated header
template_document_head = """#pragma once

// Auto-generated file. Do not edit.

#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Utility/MStringId.h"
#include "Scene/MEntity.h"
#include "Component/MComponent.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"

{component_includes}

#include <functional>
#include <unordered_map>
#include <sstream>

namespace morty
{{

// Property accessor function types
using PropertyGetFunc = std::function<MString(MComponent*)>;
using PropertySetFunc = std::function<bool(MComponent*, const MString&)>;

struct MPropertyAccessorInfo
{{
    PropertyGetFunc getter;
    PropertySetFunc setter;
    MString propertyType;
}};

// Helper functions for parsing
inline std::vector<float> ParseFloatList(const MString& str)
{{
    std::vector<float> result;
    std::stringstream ss(str);
    MString item;
    while (std::getline(ss, item, ','))
    {{
        try {{ result.push_back(std::stof(item)); }}
        catch (...) {{ return {{}}; }}
    }}
    return result;
}}

// Registry: ComponentTypeName -> PropertyName -> AccessorInfo
inline const std::unordered_map<MStringId, std::unordered_map<MStringId, MPropertyAccessorInfo>>&
GetPropertyAccessorRegistry()
{{
    static std::unordered_map<MStringId, std::unordered_map<MStringId, MPropertyAccessorInfo>> registry = {{
{registry_entries}
    }};
    return registry;
}}

}} // namespace morty
"""

# Template for each component's property accessors
template_component_entry = """        {{ MStringId("{component_name}"), {{
{property_entries}
        }} }},"""

# Template for variant property (bool, int, float, Vector types)
template_property_variant = """            {{ MStringId("{prop_name}"), {{
                // Getter
                [](MComponent* comp) -> MString {{
                    if (auto* c = comp->DynamicCast<{component_name}>()) {{
                        auto value = c->{getter}();
                        return {to_string_expr};
                    }}
                    return "";
                }},
                // Setter
                [](MComponent* comp, const MString& str) -> bool {{
                    if (auto* c = comp->DynamicCast<{component_name}>()) {{
                        {from_string_code}
                        return true;
                    }}
                    return false;
                }},
                "{prop_type}"
            }} }},"""

# Template for enum property
template_property_enum = """            {{ MStringId("{prop_name}"), {{
                // Getter
                [](MComponent* comp) -> MString {{
                    if (auto* c = comp->DynamicCast<{component_name}>()) {{
                        static const std::map<int, MString> enumNames = {enum_to_name_map};
                        auto value = static_cast<int>(c->{getter}());
                        auto it = enumNames.find(value);
                        return (it != enumNames.end()) ? it->second : std::to_string(value);
                    }}
                    return "";
                }},
                // Setter
                [](MComponent* comp, const MString& str) -> bool {{
                    if (auto* c = comp->DynamicCast<{component_name}>()) {{
                        static const std::map<MString, int> nameToEnum = {name_to_enum_map};
                        auto it = nameToEnum.find(str);
                        if (it != nameToEnum.end()) {{
                            c->{setter}(static_cast<{enum_type}>(it->second));
                            return true;
                        }}
                        // Try parsing as integer
                        try {{
                            int val = std::stoi(str);
                            c->{setter}(static_cast<{enum_type}>(val));
                            return true;
                        }} catch (...) {{}}
                    }}
                    return false;
                }},
                "{prop_type}"
            }} }},"""


class Collector(reflector_collector.Basic):

    def __init__(self):
        reflector_collector.Basic.__init__(self)
        self.m_component_table = {}
        self.m_enum_collector = None

    def set_enum_collector(self, enum_collector):
        """Set the enum collector to query enum values"""
        self.m_enum_collector = enum_collector

    def check_attr(self, attr_node) -> bool:
        return "ComponentProperty" in attr_node or "PropertyAccessor" in attr_node

    def get_property_name_from_member(self, member_name):
        """Convert m_propertyName to PropertyName"""
        if member_name.startswith('m_'):
            name = member_name[2:]
            if len(name) > 0:
                return name[0].upper() + name[1:]
        return member_name

    def parse_property_accessor(self, attr_name):
        """Parse PropertyAccessor(type,name) and return (type, name) tuple"""
        pattern = r'PropertyAccessor\(([^,]+),([^)]+)\)'
        match = re.search(pattern, attr_name)
        if match:
            return match.group(1).strip(), match.group(2).strip()
        return None, None

    def add_node(self, node, parent, _class_name):
        if _class_name not in self.m_component_table:
            self.m_component_table[_class_name] = ComponentInfo()
            self.m_component_table[_class_name].class_name = _class_name
            self.m_component_table[_class_name].properties = []

        attr_name = node.spelling

        prop_info = PropertyInfo()

        # Handle PropertyAccessor(type, name) - for derived properties like Position
        if "PropertyAccessor" in attr_name:
            prop_type, prop_name = self.parse_property_accessor(attr_name)
            if prop_type and prop_name:
                prop_info.name = prop_name
                prop_info.type = prop_type
                prop_info.property_type = PropertyType.Variant
                prop_info.getter = f"Get{prop_name}"
                prop_info.setter = f"Set{prop_name}"
                self.m_component_table[_class_name].properties.append(prop_info)
            return

        # Handle traditional ComponentProperty* attributes
        prop_info.name = parent.displayname

        if "ComponentPropertyVariant" in attr_name:
            prop_info.type = parent.type.spelling
            prop_info.property_type = PropertyType.Variant
        elif "ComponentPropertyStruct" in attr_name:
            prop_info.type = parent.type.spelling
            prop_info.property_type = PropertyType.Struct
        elif "ComponentPropertyResource" in attr_name:
            prop_info.type = property_type_parser.PropertyTypeParser.process_resource_type(attr_name)
            prop_info.property_type = PropertyType.Resource
        elif "ComponentPropertyEnum" in attr_name:
            prop_info.type = parent.type.spelling
            # Query enum values from enum collector
            if self.m_enum_collector:
                enum_values = self.m_enum_collector.get_enum_values(prop_info.type)
                # Convert to dict: {name: value, ...}
                prop_info.enum_values = {name: value for name, value in enum_values}
            else:
                prop_info.enum_values = {}
            prop_info.property_type = PropertyType.Enum

        # Get property name from member name (m_shadowType -> ShadowType)
        property_name = self.get_property_name_from_member(prop_info.name)
        prop_info.getter = f"Get{property_name}"
        prop_info.setter = f"Set{property_name}"
        self.m_component_table[_class_name].properties.append(prop_info)

    def get_to_string_expr(self, type_name):
        """Generate C++ expression to convert value to string"""
        if type_name == 'bool':
            return 'value ? "true" : "false"'
        elif type_name in ['int', 'unsigned int', 'uint32_t', 'int32_t', 'size_t']:
            return 'std::to_string(value)'
        elif type_name in ['float', 'double']:
            return 'std::to_string(value)'
        elif type_name == 'Vector2':
            return 'std::to_string(value.x) + "," + std::to_string(value.y)'
        elif type_name == 'Vector3':
            return 'std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z)'
        elif type_name == 'Vector4':
            return 'std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z) + "," + std::to_string(value.w)'
        elif type_name == 'Vector2i':
            return 'std::to_string(value.x) + "," + std::to_string(value.y)'
        elif type_name == 'Vector3i':
            return 'std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z)'
        elif type_name == 'Quaternion':
            return 'std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z) + "," + std::to_string(value.w)'
        else:
            return '"<unsupported type>"'

    def get_from_string_code(self, type_name, setter):
        """Generate C++ code to parse string and call setter"""
        if type_name == 'bool':
            return f'c->{setter}(str == "true" || str == "1");'
        elif type_name in ['int', 'int32_t']:
            return f'c->{setter}(std::stoi(str));'
        elif type_name in ['unsigned int', 'uint32_t', 'size_t']:
            return f'c->{setter}(static_cast<{type_name}>(std::stoul(str)));'
        elif type_name == 'float':
            return f'c->{setter}(std::stof(str));'
        elif type_name == 'double':
            return f'c->{setter}(std::stod(str));'
        elif type_name == 'Vector2':
            return f'''auto parts = ParseFloatList(str);
                        if (parts.size() >= 2) {{ c->{setter}(Vector2(parts[0], parts[1])); }}'''
        elif type_name == 'Vector3':
            return f'''auto parts = ParseFloatList(str);
                        if (parts.size() >= 3) {{ c->{setter}(Vector3(parts[0], parts[1], parts[2])); }}'''
        elif type_name == 'Vector4':
            return f'''auto parts = ParseFloatList(str);
                        if (parts.size() >= 4) {{ c->{setter}(Vector4(parts[0], parts[1], parts[2], parts[3])); }}'''
        elif type_name == 'Vector2i':
            return f'''auto parts = ParseFloatList(str);
                        if (parts.size() >= 2) {{ c->{setter}(Vector2i(static_cast<int>(parts[0]), static_cast<int>(parts[1]))); }}'''
        elif type_name == 'Vector3i':
            return f'''auto parts = ParseFloatList(str);
                        if (parts.size() >= 3) {{ c->{setter}(Vector3i(static_cast<int>(parts[0]), static_cast<int>(parts[1]), static_cast<int>(parts[2]))); }}'''
        elif type_name == 'Quaternion':
            return f'''auto parts = ParseFloatList(str);
                        if (parts.size() >= 4) {{ c->{setter}(Quaternion(parts[0], parts[1], parts[2], parts[3])); }}'''
        else:
            return '// Unsupported type'

    def is_supported_variant_type(self, type_name):
        """Check if this variant type is supported for get/set"""
        supported = ['bool', 'int', 'unsigned int', 'uint32_t', 'int32_t', 'size_t',
                     'float', 'double', 'Vector2', 'Vector3', 'Vector4', 'Vector2i', 'Vector3i',
                     'Quaternion']
        return type_name in supported

    def output(self, source_path):
        write_path = source_path + "/../Morty/Editor/Reflection/MComponentAccessor.gen"

        if not os.path.exists(write_path) and len(self.m_component_table) == 0:
            return

        if not os.path.exists(source_path + "/../Morty/Editor/Reflection"):
            os.makedirs(source_path + "/../Morty/Editor/Reflection")

        # Collect all component includes
        includes = set()
        for comp_name in self.m_component_table:
            includes.add(f'#include "Component/{comp_name}.h"')
        component_includes = '\n'.join(sorted(includes))

        # Build registry entries
        registry_entries = []
        for comp_name, comp_info in self.m_component_table.items():
            property_entries = []

            for prop in comp_info.properties:
                display_name = self.get_property_name_from_member(prop.name)

                if prop.property_type == PropertyType.Variant:
                    if not self.is_supported_variant_type(prop.type):
                        continue  # Skip unsupported types

                    entry = template_property_variant.format(
                        prop_name=display_name,
                        component_name=comp_name,
                        getter=prop.getter,
                        setter=prop.setter,
                        to_string_expr=self.get_to_string_expr(prop.type),
                        from_string_code=self.get_from_string_code(prop.type, prop.setter),
                        prop_type=prop.type
                    )
                    property_entries.append(entry)

                elif prop.property_type == PropertyType.Enum:
                    # Build enum maps
                    enum_to_name_pairs = []
                    name_to_enum_pairs = []
                    for name, value in sorted(prop.enum_values.items(), key=lambda x: x[1]):
                        # Remove enum prefix if present (e.g., "ENone" -> "None")
                        display_enum_name = name
                        if name.startswith('E') and len(name) > 1 and name[1].isupper():
                            display_enum_name = name[1:]
                        enum_to_name_pairs.append(f'{{{value}, "{display_enum_name}"}}')
                        name_to_enum_pairs.append(f'{{"{display_enum_name}", {value}}}')

                    entry = template_property_enum.format(
                        prop_name=display_name,
                        component_name=comp_name,
                        getter=prop.getter,
                        setter=prop.setter,
                        enum_type=prop.type,
                        enum_to_name_map='{' + ', '.join(enum_to_name_pairs) + '}',
                        name_to_enum_map='{' + ', '.join(name_to_enum_pairs) + '}',
                        prop_type=prop.type
                    )
                    property_entries.append(entry)

                # Skip Resource and Struct types for now (complex)

            if property_entries:
                comp_entry = template_component_entry.format(
                    component_name=comp_name,
                    property_entries='\n'.join(property_entries)
                )
                registry_entries.append(comp_entry)

        output = template_document_head.format(
            component_includes=component_includes,
            registry_entries='\n'.join(registry_entries)
        )

        with open(write_path, "w") as fo:
            fo.write(output)
