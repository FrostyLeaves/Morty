#coding:UTF-8
import sys, getopt, os, time
import clang.cindex


def record_reflector(headerFile):
    index = clang.cindex.Index.create()
    tu = index.parse(headerFile)
    tu.


def main(argv):

    if len(argv) < 1:
        return

    path = argv[0]
    filterSuffix = [".h"]

    for mainDir, _, fileNames in os.walk(path):

        for fileName in fileNames:
            fullPath = os.path.join(mainDir, fileName)
            suffix = os.path.splitext(fullPath)[1]

            if suffix in filterSuffix:
                record_reflector(fullPath)

if __name__ == "__main__":
   main(sys.argv[1:])