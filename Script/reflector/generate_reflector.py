#coding:UTF-8
import sys, getopt, os, time
import datetime
import clang.cindex # type: ignore
from clang.cindex import * # type: ignore

registed_attr_name_list = [
    "test_attr"
]

parse_empty = "empty.cpp"

def check_attr(node):
    
    if node.spelling != '' and node.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
        if node.spelling in registed_attr_name_list:
            return True
    return False

def walk(node, parent, deep):
    for child_node in node.get_children():
        if (check_attr(child_node)):
            for i in range(0, deep): print('  ', end = '')
            print ('name: %s, parent: %s' % (node.spelling or node.displayname, parent.displayname))
            for i in range(0, deep): print('  ', end = '')
            print ('kind: %s' % (node.kind))
            for i in range(0, deep): print('  ', end = '')
            print ('type: %s' % (node.type.spelling))
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
    
    path = argv[0]
    build_dir = argv[1]
    clang.cindex.Config.set_library_file(argv[2]) #clang path

    filterSuffix = [".h"]
    clang_index = clang.cindex.Index.create()

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
    print("args: ", file_args)
    for mainDir, _, fileNames in os.walk(path):

        for fileName in fileNames:
            fullPath = os.path.join(mainDir, fileName)
            suffix = os.path.splitext(fullPath)[1]
            if suffix in filterSuffix:
                compile_source += '#include \"'+fullPath+'\"\n'
    clang_parse(compile_source, clang_index, file_args)

if __name__ == "__main__":
    start_time = time.time()
    main(sys.argv[1:])
    end_time = time.time()
    print("total time: %f s" % (end_time - start_time))