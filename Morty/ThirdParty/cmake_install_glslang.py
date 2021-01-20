import os
import sys
import shutil


WORK_PATH = os.getcwd()
GLSLANG_PATH = WORK_PATH + "/SPIRV-Cross/external/glslang"
GLSLANG_INSTALL_PATH = WORK_PATH + "/installs/glslang"

GLSLANG_BUILD_PATH = WORK_PATH + "/glslang-Build"

def build_glslang_for_windows():

    CMAKE_PATH = "cmake"


    if not os.path.exists(GLSLANG_BUILD_PATH):
            os.makedirs(GLSLANG_BUILD_PATH)
            
    os.chdir(GLSLANG_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' --G "Visual Studio 16 Win64" ' +
      " -DCMAKE_INSTALL_PREFIX=" + GLSLANG_INSTALL_PATH +
      " -DCMAKE_DEBUG_POSTFIX=d" +
      " " + GLSLANG_PATH)


    if not os.path.exists(GLSLANG_INSTALL_PATH): 
        os.makedirs(GLSLANG_INSTALL_PATH)
        
    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    #os.system(CMAKE_PATH + " --build ./ --target install --config Release")
    

    os.chdir(WORK_PATH)


    shutil.rmtree(GLSLANG_BUILD_PATH)


build_glslang_for_windows()

