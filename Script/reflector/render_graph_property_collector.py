import os
import reflector_collector

template_document_head = """#include "Render/RenderGraph/MRenderGraph.h"
#include "Widget/RenderGraph/EditRenderTaskNodeBase.h"
#include "Widget/RenderGraph/MRenderGraphNodeList.h"

#define PROPERTY_VALUE_EDIT(NODE, KEY_NAME, TYPE, VALUE_NAME)                                                  \
    ShowValueBegin(KEY_NAME);                                                                                          \
    if (Edit##TYPE(NODE->VALUE_NAME)) { renderNode->GetRenderGraph()->RequireCompile(); }                              \
    ShowValueEnd();

using namespace morty;
"""

template_document_tail = """
"""

template_node_edit_head = """
#include "Render/RenderNode/{0}.h"
class Property{0} : public EditRenderTaskNodeBase
{{
public:

    [[nodiscard]] const MType* GetNodeType() const override
    {{
        return {0}::GetClassType();
    }}

    void EditRenderTaskNode(MEngine* pEngine, MRenderTaskNode* pRenderNode) override
    {{
        BindEngine(pEngine);

        auto renderNode = pRenderNode->DynamicCast<{0}>();
        if (!renderNode)
        {{
            return;
        }}
        
        if (ShowNodeBegin("{0}"))
        {{
"""

template_property = """             PROPERTY_VALUE_EDIT(renderNode, "{0}", {1}, {0});
"""

template_node_edit_tail = """
            ShowNodeEnd();
        }
    }
};
"""



template_document_factory_head = """
const std::unordered_map<MStringId, MRenderGraphNodeList::EditCreateFunc> MRenderGraphNodeList::EditFactory = {      
"""
template_document_factory_tail = """
};
"""

class ReflectorAttr:
    class_name = ""
    property_name = []
    property_type = []

class Collector(reflector_collector.Basic):
    
    m_node_table = {}

    m_replace_table = {
        "std::shared_ptr<MResource>": "MResource",
        "std::shared_ptr<MMaterialResource>": "MMaterialResource",
    }
    
    def __init__(self):
        reflector_collector.Basic.__init__(self)
        self.m_node_list = []

    def check_attr(self, attr_node) -> bool:
        return attr_node == "RenderNodeProperty"

    def add_node(self, node, parent, _class_name):

        if _class_name not in self.m_node_table:
            self.m_node_table[_class_name] = ReflectorAttr()
            self.m_node_table[_class_name].class_name = _class_name
            self.m_node_table[_class_name].property_name = []
            self.m_node_table[_class_name].property_type = []

        property_name = parent.displayname
        property_type = parent.type.spelling

        if property_type in self.m_replace_table:
            property_type = self.m_replace_table[property_type]

        self.m_node_table[_class_name].property_name.append(property_name)
        self.m_node_table[_class_name].property_type.append(property_type)
    
    
    def output(self, source_path):
        write_path = source_path + "/../Editor/Reflection/MRenderGraphNodeProperty.gen"
        if not os.path.exists(write_path) and len(self.m_node_table) == 0:
            return;
    
        if not os.path.exists(source_path + "/../Editor/Reflection"):
            os.makedirs(source_path + "/../Editor/Reflection")
    
        fo = open(write_path, "w")
        fo.write(template_document_head)

        output_string = ""
        output_factory = ""
        for iter in self.m_node_table:
            node : ReflectorAttr = self.m_node_table[iter]

            output_string += template_node_edit_head.format(node.class_name)

            for prop_idx in range(0, len(node.property_name)):
                output_string += template_property.format(node.property_name[prop_idx], node.property_type[prop_idx], node.property_name[prop_idx])
    
            output_string += template_node_edit_tail
        
            output_factory += "    {{ MStringId(\"{0}\"), [](){{ return static_cast<EditRenderTaskNodeBase*>(new Property{0}()); }} }},\n".format(node.class_name)

        
        fo.write( output_string );

        
        fo.write(template_document_tail)




        fo.write(template_document_factory_head)
        fo.write(output_factory)
        fo.write(template_document_factory_tail)



        fo.close()
        return;