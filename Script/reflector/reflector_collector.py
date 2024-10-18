
class ReflectorAttr:

    owner_name = ""
    attr_name = ""


class Basic:

    m_node_list = []


    def check_attr(self, attr_node) -> bool:
        return False
    
    def add_node(self, node, parent):

        attr = ReflectorAttr()
        attr.owner_name = parent.displayname
        attr.attr_name = node.displayname

        self.m_node_list.append( attr )
    
    def output(self, source_path):
        return;