import os
import reflector_collector


template_document_head = """
using namespace morty;

const std::vector<MStringId> MRenderGraphNodeList::Names = {
"""

template_document_tail = """
};
"""

class Collector(reflector_collector.Basic):
    
    def __init__(self):
        reflector_collector.Basic.__init__(self)

    def check_attr(self, attr_node) -> bool:
        return attr_node == "RenderGraphNode"
    
    def output(self, source_path):
        write_path = source_path + "/../Editor/Reflection/MRenderGraphNodeList.gen"
        if not os.path.exists(write_path) and len(self.m_node_list) == 0:
            return;
    
        fo = open(write_path, "w")

        output_names = ""
        for iter in self.m_node_list:
            node : reflector_collector.ReflectorAttr = iter

            output_names += "    MStringId(\"{}\"),\n".format(node.owner_name)
        
        
        fo.write(template_document_head)
        fo.write(output_names)
        fo.write(template_document_tail)


        fo.close()
        return;