#coding:UTF-8
import sys, getopt, os, time

svExeFileName = "glslangValidator.exe"

svVertexCmds = " -S vert -e VS -V {0} -o {1} -D {2}"
svPixelCmds = " -S frag -e PS -V {0} -o {1} -D {2}"

macros = [
"MBONES_PER_VERTEX",
"MBONES_MAX_NUMBER",
"MPOINT_LIGHT_MAX_NUMBER",
"MSPOT_LIGHT_MAX_NUMBER",
"MSHADOW_TEXTURE_SIZE",
"MPOINT_LIGHT_PIXEL_NUMBER",
"MSPOT_LIGHT_PIXEL_NUMBER",
]

def getMacros():
    result = ""
    for macro in macros:
        result = result + " --define-macro " + macro
    
    return result

def getShaderType(fileName):
    if -1 != fileName.rfind('.mvs'):
        return 1
    elif -1 != fileName.rfind('.mps'):
        return 2
    return 0

def getCmds(inputFile, outputFile):
    eType = getShaderType(inputFile)
    if 1 == eType:
        return svVertexCmds.format(getMacros(), outputFile, inputFile)
    elif 2 == eType:
        return svPixelCmds.format(getMacros(), outputFile, inputFile)
    return ""

def main(argv):

    if len(argv) != 2:
        print("len(argv) == " + str(len(argv)))
        return

    path = argv[0]
    sourceFileName = argv[0] + "/" + argv[1]
    
    targetFileName = argv[0] + "/" + argv[1] + "v"
    



    if (not os.path.isdir(path)) or not os.path.exists(path):
        os.makedirs(path)

    cmd = svExeFileName + getCmds(sourceFileName, targetFileName)
    
    print("run:  " + cmd)
    r_v = os.system(cmd) 
    print (r_v )


if __name__ == "__main__":
   main(sys.argv[1:])
   input('finished.')