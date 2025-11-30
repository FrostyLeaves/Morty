import os
import re
import reflector_collector
import clang.cindex

class EnumValueInfo:
    """Stores information about a single enum value"""
    def __init__(self, name, value):
        self.name = name
        self.value = value

class EnumInfo:
    """Stores information about an enum type"""
    def __init__(self, name):
        self.name = name
        self.values = []  # List of EnumValueInfo

class Collector(reflector_collector.Basic):
    """Collects enums marked with MORTY_ENUM attribute"""

    def __init__(self):
        reflector_collector.Basic.__init__(self)
        self.m_enum_table = {}  # key: enum name, value: EnumInfo

    def check_attr(self, attr_node) -> bool:
        return "MORTY_ENUM" in attr_node

    def add_node(self, node, parent, _class_name):
        """
        Called when a MORTY_ENUM attribute is found.
        parent should be the enum declaration node.
        """
        # The parent node should be the enum declaration
        if parent.kind != clang.cindex.CursorKind.ENUM_DECL:
            return

        enum_name = parent.spelling
        if not enum_name:
            # For nested enums, use displayname
            enum_name = parent.displayname

        # Create or get enum info
        if enum_name not in self.m_enum_table:
            self.m_enum_table[enum_name] = EnumInfo(enum_name)

        enum_info = self.m_enum_table[enum_name]

        # Collect enum values
        for child in parent.get_children():
            if child.kind == clang.cindex.CursorKind.ENUM_CONSTANT_DECL:
                enum_value_name = child.spelling
                enum_value = child.enum_value
                enum_info.values.append(EnumValueInfo(enum_value_name, enum_value))

    def get_enum_values(self, enum_type_name):
        """
        Returns a list of (name, value) tuples for the given enum type.
        Returns empty list if enum not found.
        """
        # Handle qualified names like "MRenderMeshComponent::MEShadowType"
        # Try full name first
        if enum_type_name in self.m_enum_table:
            return [(v.name, v.value) for v in self.m_enum_table[enum_type_name].values]

        # Try without class prefix
        if '::' in enum_type_name:
            simple_name = enum_type_name.split('::')[-1]
            if simple_name in self.m_enum_table:
                return [(v.name, v.value) for v in self.m_enum_table[simple_name].values]

        return []

    def output(self, source_path):
        """
        Output enum reflection information.
        For debugging purposes, we'll write a summary file.
        """
        write_path = source_path + "/../Morty/Editor/Reflection/MEnumReflection.gen"

        if len(self.m_enum_table) == 0:
            return

        if not os.path.exists(source_path + "/../Morty/Editor/Reflection"):
            os.makedirs(source_path + "/../Morty/Editor/Reflection")

        with open(write_path, "w") as fo:
            fo.write("#pragma once\n\n")
            fo.write("// Auto-generated enum reflection information\n")
            fo.write("// Total enums collected: {}\n\n".format(len(self.m_enum_table)))

            for enum_name, enum_info in self.m_enum_table.items():
                fo.write("// Enum: {}\n".format(enum_name))
                for value_info in enum_info.values:
                    fo.write("//   {} = {}\n".format(value_info.name, value_info.value))
                fo.write("\n")
