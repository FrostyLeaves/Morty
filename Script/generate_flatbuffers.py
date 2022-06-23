#coding:UTF-8
import sys, getopt, os, time

Flatc_Path = "../ThirdParty/installs/flatbuffers/bin/flatc.exe"

Root_Path = [
    "../Morty/Core",
    "../Morty/Editor",
]

def headerDocument(className):
    return headerTemaplateString.format(className.upper(), className, time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), "Pobrecito" )

def scourceDocument(className):
    return sourceTemaplateString.format(className)

def gen_all_fbs(path, include_path, output_path, flat_exec):
    files_list = os.listdir(path)

    for file in files_list:
        cur_path = os.path.join(path, file)

        if os.path.isdir(cur_path):
            gen_all_fbs(cur_path, include_path, output_path, flat_exec)
        elif cur_path.endswith("fbs"):
            cmd =(flat_exec +
            ' --cpp' +
            ' -I ' + include_path +
            ' -o ' + output_path + '/' + 
            ' ' + cur_path + 
            ' --gen-object-api')
            print("cmd: ", cmd)
            os.system(cmd)
            
def main(argv):

    flat_exec = os.path.abspath('../ThirdParty/installs/flatbuffers/bin/flatc.exe')

    for path in Root_Path:
        abs_path = os.path.abspath(path)
        gen_all_fbs(abs_path, abs_path, abs_path + "/Flatbuffer", flat_exec)


if __name__ == "__main__":
   main([])
   input('finished.')