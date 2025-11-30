import os
import re
from enum import Enum
import reflector_collector
import property_type_parser

template_document_head = """#pragma once

#include "Engine/MEngine.h"
#include "Property/MComponentProperty.h"
#include "Scene/MEntity.h"

using namespace morty;
"""

template_document_tail = """
"""

template_component_property_head = """
#include "Component/{0}.h"
class Property{0} : public MComponentProperty
{{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {{
        MORTY_UNUSED(editor);
        m_editProperty.BindEngine(pEntity->GetEngine());

        if (auto* component = pEntity->GetComponent<{0}>())
        {{
            if (m_editProperty.ShowNodeBegin("{0}"))
            {{
"""
template_property = """				PROPERTY_VALUE_GET_SET_EDIT(component, "{0}", {1}, {2}, {3});
"""
template_property_struct = """				PROPERTY_NODE_EDIT(component, "{0}", {1}, {2}, {3});
"""

template_property_resource = """				PROPERTY_RESOURCE_GET_SET_EDIT(component, "{0}", {1}, {2}, {3});
"""

template_property_enum = """				static std::map<MString, int> Enum{0} = {4};
				PROPERTY_ENUM_GET_SET_EDIT(component, "{0}", {1}, {2}, {3}, Enum{0});
"""

template_component_property_tail = """
				m_editProperty.ShowNodeEnd();
            }
        }
    }
};
"""

template_document_factory_head = """
const std::unordered_map<MStringId, MComponentPropertyList::PropertyCreateFunc> MComponentPropertyList::EditFactory = {
"""

template_document_factory_tail = """
};
"""
class PropertyType(Enum):
    Variant = 1
    Enum = 2
    Resource = 3
    Struct = 4

class PropertyInfo:
    name = ""
    type = ""
    getter = ""
    setter = ""
    property_type: PropertyType = PropertyType.Variant
    enum_values = {}

class ComponentInfo:
    class_name = ""
    properties = []

class Collector(reflector_collector.Basic):

    m_component_table = {}
    m_enum_collector = None

    def __init__(self):
        reflector_collector.Basic.__init__(self)
        self.m_component_table = {}
        self.m_enum_collector = None

    def set_enum_collector(self, enum_collector):
        """Set the enum collector to query enum values"""
        self.m_enum_collector = enum_collector

    def check_attr(self, attr_node) -> bool:
        return "ComponentProperty" in attr_node

    def get_property_name_from_member(self, member_name):
        """Convert m_propertyName to PropertyName"""
        if member_name.startswith('m_'):
            name = member_name[2:]
            if len(name) > 0:
                return name[0].upper() + name[1:]
        return member_name

    def add_node(self, node, parent, _class_name):
        if _class_name not in self.m_component_table:
            self.m_component_table[_class_name] = ComponentInfo()
            self.m_component_table[_class_name].class_name = _class_name
            self.m_component_table[_class_name].properties = []

        attr_name = node.spelling

        prop_info = PropertyInfo()
        prop_info.name = parent.displayname
        if ("ComponentPropertyVariant" in attr_name):
            prop_info.type = parent.type.spelling
            prop_info.property_type = PropertyType.Variant
        elif ("ComponentPropertyStruct" in attr_name):
            prop_info.type = parent.type.spelling
            prop_info.property_type = PropertyType.Struct
        elif ("ComponentPropertyResource" in attr_name):
            prop_info.type = property_type_parser.PropertyTypeParser.process_resource_type(attr_name)
            prop_info.property_type = PropertyType.Resource
        elif ("ComponentPropertyEnum" in attr_name):
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
    
    def enum_to_array(self, enum_values):
        """Convert enum values dict to C++ map/array format with name-value pairs"""
        if not enum_values:
            return "{}"

        # Create array of {name, value} pairs
        # Format: {{"EnumName1", 0}, {"EnumName2", 1}, ...}
        enum_pairs = []
        # Sort by value to maintain enum order
        for name, value in sorted(enum_values.items(), key=lambda x: x[1]):
            # Remove enum prefix if present (e.g., "ENone" -> "None")
            display_name = name
            if name.startswith('E') and len(name) > 1 and name[1].isupper():
                display_name = name[1:]
            enum_pairs.append(f'{{"{display_name}", {value}}}')

        return "{" + ", ".join(enum_pairs) + "}"

    def generate_property_code(self, component_name, prop_info):
        type_category = property_type_parser.PropertyTypeParser.process_property_type(prop_info.type)

        # Get display name from member variable name
        display_name = self.get_property_name_from_member(prop_info.name)


        if (prop_info.property_type == PropertyType.Enum):
            return template_property_enum.format(
                display_name,
                type_category,
                prop_info.getter,
                prop_info.setter,
                self.enum_to_array(prop_info.enum_values)
            )
        elif (prop_info.property_type == PropertyType.Resource):
            return template_property_resource.format(
                display_name,
                type_category,
                prop_info.getter,
                prop_info.setter
            )
        elif (prop_info.property_type == PropertyType.Struct):
            return template_property_struct.format(
                display_name,
                type_category,
                prop_info.getter,
                prop_info.setter
            )
        return template_property.format(
            display_name,
            type_category,
            prop_info.getter,
            prop_info.setter
        )

        return ""

    def output(self, source_path):
        write_path = source_path + "/../Morty/Editor/Reflection/MComponentProperty.gen"
        if not os.path.exists(write_path) and len(self.m_component_table) == 0:
            return

        if not os.path.exists(source_path + "/../Morty/Editor/Reflection"):
            os.makedirs(source_path + "/../Morty/Editor/Reflection")

        fo = open(write_path, "w")
        fo.write(template_document_head)

        output_string = ""
        output_factory = ""

        for component_name in self.m_component_table:
            component_info = self.m_component_table[component_name]

            output_string += template_component_property_head.format(component_info.class_name)

            for prop_info in component_info.properties:
                prop_code = self.generate_property_code(component_info.class_name, prop_info)
                if prop_code:
                    output_string += prop_code

            output_string += template_component_property_tail

            output_factory += f'    {{ MStringId("{component_info.class_name}"), [](){{ return static_cast<MComponentProperty*>(new Property{component_info.class_name}()); }} }},\n'

        fo.write(output_string)
        fo.write(template_document_tail)

        fo.write(template_document_factory_head)
        fo.write(output_factory)
        fo.write(template_document_factory_tail)

        fo.close()
        return
