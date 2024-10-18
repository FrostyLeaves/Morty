import os
import reflector_collector


template_document = """#include "MRenderGraphNodeList.h"
using namespace morty;

const std::vector<MStringId> MRenderGraphNodeList::Names = {{
    {}
}};
"""


class Collector(reflector_collector.Basic):

    def check_attr(self, attr_node) -> bool:
        return attr_node == "RenderGraphNode"
    
    def output(self, source_path):
        write_path = source_path + "/Reflection/MRenderGraphNodeList.gen"
        if not os.path.exists(write_path) and len(self.m_node_list) == 0:
            return;
    
        fo = open(write_path, "w")

        output_string = ""
        for iter in self.m_node_list:
            node : reflector_collector.ReflectorAttr = iter

            output_string += "MStringId(\"{}\")".format(node.owner_name)

        
        fo.write( template_document.format(output_string) );
        fo.close()
        return;