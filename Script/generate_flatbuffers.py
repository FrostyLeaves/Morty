#coding:UTF-8
import sys, getopt, os, time, shutil

Root_Path = [
    "../Morty/Core",
    "../Morty/Render",
    "../Morty/Editor",
]

#create from chatGPT
def find_executable(name, path=None):
    if path is None:
        path = os.environ.get('PATH', '').split(os.pathsep)

    if sys.platform == 'win32':
        name = f'{name}.exe'

    for dir in path:
        for root, dirs, files in os.walk(dir):
            if name in files:
                file_path = os.path.join(root, name)
                if os.access(file_path, os.X_OK):
                    absolute_path = os.path.abspath(file_path)
                    return absolute_path

    return None

def gen_all_fbs(path, include_path, output_path, flat_exec):
    files_list = os.listdir(path)

    for file in files_list:
        cur_path = os.path.join(path, file)

        if os.path.isdir(cur_path):
            gen_all_fbs(cur_path, include_path, output_path, flat_exec)
        elif cur_path.endswith("fbs"):
            cmd =(flat_exec +
            ' --cpp' +
            include_path +
            ' -o ' + output_path + '/' + 
            ' ' + cur_path + 
            ' --gen-object-api')
            print("cmd: ", cmd)
            os.system(cmd)
            
def main(argv):

    flat_exec = find_executable('flatc', path=['../ThirdParty/vcpkg/packages'])
    if flat_exec == None:
        print("error: can`t find flatc program.")
    print('faltc path: ', flat_exec)

    include_path = ''
    for path in Root_Path:
        abs_path = os.path.abspath(path)
        include_path += ' -I ' + abs_path + ' '
    
    
    for path in Root_Path:
        abs_path = os.path.abspath(path)
        gen_all_fbs(abs_path, include_path, abs_path + "/Flatbuffer", flat_exec)


if __name__ == "__main__":
   main([])
   input('finished.')