
class ReflectorAttr:

    owner_name = ""
    attr_name = ""
    class_name = ""


class Basic:

    m_node_list = []

    def __init__(self):
        self.m_node_list = []

    def check_attr(self, attr_node) -> bool:
        return False
    
    def add_node(self, node, parent, _class_name):

        attr = ReflectorAttr()
        attr.owner_name = parent.displayname
        attr.attr_name = node.displayname
        attr.class_name = _class_name

        self.m_node_list.append( attr )
    
    def output(self, source_path):
        return;