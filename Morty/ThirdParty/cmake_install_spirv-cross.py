import os
import sys
import shutil


WORK_PATH = os.getcwd()
SPIRV_CROSS_PATH = WORK_PATH + "/SPIRV-Cross"
SPIRV_CROSS_INSTALL_PATH = WORK_PATH + "/installs/SPIRV-Cross"

SPIRV_CROSS_BUILD_PATH = WORK_PATH + "/SPIRV-Cross-Build"

def build_SPIRV_Cross_for_windows():

    CMAKE_PATH = "cmake"


    if not os.path.exists(SPIRV_CROSS_BUILD_PATH):
            os.makedirs(SPIRV_CROSS_BUILD_PATH)
            
    os.chdir(SPIRV_CROSS_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' --G "Visual Studio 16 Win64" ' +
      " -DCMAKE_INSTALL_PREFIX=" + SPIRV_CROSS_INSTALL_PATH +
      " " + SPIRV_CROSS_PATH)


    if not os.path.exists(SPIRV_CROSS_INSTALL_PATH): 
        os.makedirs(SPIRV_CROSS_INSTALL_PATH)
        
    os.system(CMAKE_PATH + " --build ./ --target INSTALL --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target INSTALL --config Release")
    

    os.chdir(WORK_PATH)


    shutil.rmtree(SPIRV_CROSS_BUILD_PATH)


os.chdir(SPIRV_CROSS_PATH)
os.system("checkout_glslang_spirv_tools.sh")
os.system("build_glslang_spirv_tools.sh")
os.chdir(WORK_PATH)



build_SPIRV_Cross_for_windows()

