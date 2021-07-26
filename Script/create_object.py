#coding:UTF-8
import sys, getopt, os, time

headerTemaplateString = """/**
 * @File         {1}
 * 
 * @Created      {2}
 *
 * @Author       {3}
**/

#ifndef _M_{0}_H_
#define _M_{0}_H_
#include "MGlobal.h"
#include "MObject.h"

class MORTY_API {1} : public MObject
{{
public:
	MORTY_CLASS({1});
    {1}();
    virtual ~{1}();

public:

private:
}};

#endif
"""

sourceTemaplateString = """#include "{0}.h"

MORTY_CLASS_IMPLEMENT({0}, MObject)

{0}::{0}()
    : MObject()
{{
}}

{0}::~{0}()
{{
}}

"""

def headerDocument(className):
    return headerTemaplateString.format(className.upper(), className, time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), "Pobrecito" )

def scourceDocument(className):
    return sourceTemaplateString.format(className)

def main(argv):

    if len(argv) != 2:
        print("len(argv) == " + str(len(argv)))
        return

    path = argv[0]
    className = argv[1]

    if (not os.path.isdir(path)) or not os.path.exists(path):
        os.makedirs(path)

    fp = open(path + "/" + className + ".h", "w")
    fp.write(headerDocument(className))
    fp.close()

    fp = open(path + "/" + className + ".cpp", "w")
    fp.write(scourceDocument(className))
    fp.close()

if __name__ == "__main__":
   main(sys.argv[1:])
   input('finished.')