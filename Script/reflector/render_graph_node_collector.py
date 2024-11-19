import os
import reflector_collector


template_document_head = """
using namespace morty;

const std::vector<MStringId> MRenderGraphNodeList::Names = {
"""

template_document_tail = """
};
"""

class ReflectorAttr:

    owner_name = ""
    owner_type = ""
    attr_name = ""
    class_name = ""


class Collector(reflector_collector.Basic):
    
    m_node_list = []
    
    def __init__(self):
        reflector_collector.Basic.__init__(self)
        self.m_node_list = []

    def check_attr(self, attr_node) -> bool:
        return attr_node == "RenderGraphNode"
    

    def add_node(self, node, parent, _class_name):
        attr = ReflectorAttr()
        attr.owner_name = parent.displayname
        attr.owner_type = parent.type.spelling
        attr.attr_name = node.displayname
        attr.class_name = _class_name

        self.m_node_list.append( attr )

    def output(self, source_path):
        write_path = source_path + "/../Editor/Reflection/MRenderGraphNodeList.gen"
        if not os.path.exists(write_path) and len(self.m_node_list) == 0:
            return;

        if not os.path.exists(source_path + "/../Editor/Reflection"):
            os.makedirs(source_path + "/../Editor/Reflection")
    
        fo = open(write_path, "w")

        output_names = ""
        for iter in self.m_node_list:
            node : ReflectorAttr = iter

            output_names += "    MStringId(\"{}\"),\n".format(node.owner_name)
        
        
        fo.write(template_document_head)
        fo.write(output_names)
        fo.write(template_document_tail)


        fo.close()
        return;