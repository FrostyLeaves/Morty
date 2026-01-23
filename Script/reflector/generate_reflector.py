#coding:UTF-8
import sys, getopt, os, time
import datetime
import clang.cindex # type: ignore
from clang.cindex import * # type: ignore
#clang.cindex.Config.set_library_file( 'libclang.dll' ) #clang path

import render_graph_node_collector
import render_graph_property_collector
import component_property_collector
import component_accessor_collector
import enum_collector

registed_attr_name_list = [
    "test_attr"
]

parse_empty = "empty.cpp"

collector_list = []

def walk(node, parent, class_name, deep):

    if node.kind == clang.cindex.CursorKind.CLASS_DECL:
        class_name = node.displayname

    if node.spelling != '' and node.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
        for collector in collector_list:
            if collector.check_attr(node.spelling):
                collector.add_node(node, parent, class_name)
        
    for child_node in node.get_children():
        walk(child_node, node, class_name, deep + 1)
    
    #reset
    class_name = ""


def clang_parse(compile_source, index, args):
    translationUnit = index.parse(parse_empty, args=args, unsaved_files= [(parse_empty, compile_source)])

    # Only check for errors, ignore warnings
    errors = [d for d in translationUnit.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
    assert(len(errors) == 0)

    rootNode = translationUnit.cursor
    for child_node in rootNode.get_children():
        walk(child_node, rootNode, "", 0)


def main(argv):

    if len(argv) < 1:
        return
    
    source_path = argv[0]
    build_dir = argv[1]

    filterSuffix = [".h"]
    clang_index = clang.cindex.Index.create()


    collector_list.append(render_graph_node_collector.Collector())
    collector_list.append(render_graph_property_collector.Collector())

    # Create enum collector first
    enum_col = enum_collector.Collector()
    collector_list.append(enum_col)

    # Create component property collector and link it to enum collector
    component_prop_col = component_property_collector.Collector()
    component_prop_col.set_enum_collector(enum_col)
    collector_list.append(component_prop_col)

    # Create component accessor collector for get/set commands
    accessor_col = component_accessor_collector.Collector()
    accessor_col.set_enum_collector(enum_col)
    collector_list.append(accessor_col)


    compdb = clang.cindex.CompilationDatabase.fromDirectory(build_dir)
    commands = compdb.getCompileCommands(parse_empty)
    file_args = []
    for command in commands:
        for argument in command.arguments:
            file_args.append(argument)

    file_args.pop(0)
    file_args.pop(len(file_args) - 1)
    file_args.pop(len(file_args) - 1)

    # Disable MSVC STL version check for older Clang versions
    file_args.append("-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH")


    compile_source = ""
    for mainDir, _, fileNames in os.walk(source_path):

        for fileName in fileNames:
            fullPath = os.path.join(mainDir, fileName)
            suffix = os.path.splitext(fullPath)[1]
            if suffix in filterSuffix:
                compile_source += '#include \"'+fullPath+'\"\n'
    clang_parse(compile_source, clang_index, file_args)


    for collector in collector_list:
        collector.output(source_path)

if __name__ == "__main__":
    start_time = time.time()
    main(sys.argv[1:])
    end_time = time.time()
    print("total time: %f s" % (end_time - start_time))