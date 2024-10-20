import os
import reflector_collector

template_document_head = """#include "Render/RenderGraph/MRenderGraph.h"
#include "Widget/RenderGraph/EditRenderTaskNodeBase.h"
#include "Widget/RenderGraph/MRenderGraphNodeList.h"

#define PROPERTY_VALUE_EDIT(NODE, KEY_NAME, TYPE, VALUE_NAME)                                                          \
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

    void EditRenderTaskNode(MRenderTaskNode* pRenderNode) override
    {{

        auto renderNode = pRenderNode->DynamicCast<{0}>();
        if (!renderNode)
        {{
            return;
        }}
        
        if (ShowNodeBegin("{0}"))
        {{
            ImGui::Columns(2);
"""

template_property = """PROPERTY_VALUE_EDIT(renderNode, "{0}", {1}, {0});
"""

template_node_edit_tail = """
            ImGui::Columns(1);
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



class Collector(reflector_collector.Basic):
    
    def __init__(self):
        reflector_collector.Basic.__init__(self)

    def check_attr(self, attr_node) -> bool:
        return attr_node == "RenderNodeProperty"
    
    def output(self, source_path):
        write_path = source_path + "/../Editor/Reflection/MRenderGraphNodeProperty.gen"
        if not os.path.exists(write_path) and len(self.m_node_list) == 0:
            return;
    
        fo = open(write_path, "w")
        fo.write(template_document_head)

        output_string = ""
        output_factory = ""
        for iter in self.m_node_list:
            node : reflector_collector.ReflectorAttr = iter

            print("owner_name: ", node.owner_name)
            print("attr_name:", node.attr_name)
            print("class_name:", node.class_name)

            output_string += template_node_edit_head.format(node.class_name)
            output_string += template_property.format(node.owner_name, "bool", node.owner_name)
            output_string += template_node_edit_tail
        
            output_factory += "    {{ MStringId(\"{0}\"), [](){{ return static_cast<EditRenderTaskNodeBase*>(new Property{0}()); }} }},\n".format(node.class_name)

        
        fo.write( output_string );

        
        fo.write(template_document_tail)




        fo.write(template_document_factory_head)
        fo.write(output_factory)
        fo.write(template_document_factory_tail)



        fo.close()
        return;