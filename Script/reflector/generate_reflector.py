#coding:UTF-8
import sys, getopt, os, time
import datetime
from distutils.spawn import find_executable
import clang.cindex # type: ignore
from clang.cindex import * # type: ignore
#clang.cindex.Config.set_library_file( 'libclang.dll' ) #clang path

import render_graph_node_collector

registed_attr_name_list = [
    "test_attr"
]

parse_empty = "empty.cpp"

collector_list = []

def walk(node, parent, deep):
    if node.spelling != '' and node.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
        for collector in collector_list:
            if collector.check_attr(node.spelling):
                collector.add_node(node, parent)
        
    for child_node in node.get_children():
        walk(child_node, node, deep + 1)


def clang_parse(compile_source, index, args):
    translationUnit = index.parse(parse_empty, args=args, unsaved_files= [(parse_empty, compile_source)])

    if len(translationUnit.diagnostics) > 0:
        for d in translationUnit.diagnostics:
            print(d)
        assert(len(translationUnit.diagnostics) == 0)
    else:
        rootNode = translationUnit.cursor
        for child_node in rootNode.get_children():
            walk(child_node, rootNode, 0)


def main(argv):

    if len(argv) < 1:
        return
    
    source_path = argv[0]
    build_dir = argv[1]

    filterSuffix = [".h"]
    clang_index = clang.cindex.Index.create()


    collector_list.append(render_graph_node_collector.Collector())


    compdb = clang.cindex.CompilationDatabase.fromDirectory(build_dir)
    commands = compdb.getCompileCommands(parse_empty)
    file_args = []
    for command in commands:
        for argument in command.arguments:
            file_args.append(argument)

    file_args.pop(0)
    file_args.pop(len(file_args) - 1)
    file_args.pop(len(file_args) - 1)


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