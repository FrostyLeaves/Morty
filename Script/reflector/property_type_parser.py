import os
import re





class PropertyTypeParser:

    def process_property_type(property_type):
        # std::shared_ptr<T>
        shared_ptr_pattern = r'std::shared_ptr<(.+)Resource>'
        match = re.match(shared_ptr_pattern, property_type)
        if match:
            # 提取尖括号内的类型
            return match.group(1) + "Resource"

        # Handle nested enum types like "ClassName::EnumType"
        # Extract just the enum type name for display
        if '::' in property_type:
            # Return the full qualified name
            return property_type

        return property_type
    

    def process_resource_type(attr_name):
        # 匹配括号中的内容
        pattern = r'ComponentPropertyResource\(([^)]+)\)'
        match = re.search(pattern, attr_name)
        if match:
            return match.group(1)
        return None