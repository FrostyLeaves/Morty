import os
import sys
import shutil


WORK_PATH = os.getcwd()
SPIRV_CROSS_PATH = WORK_PATH + "/SPIRV-Cross"
SPIRV_CROSS_INSTALL_PATH = WORK_PATH + "/installs/SPIRV-Cross"

SPIRV_CROSS_BUILD_PATH = WORK_PATH + "/SPIRV-Cross-Build"

def build_SPIRV_Cross_for_windows():


    os.chdir(SPIRV_CROSS_PATH)
    os.system("./checkout_glslang_spirv_tools.sh")
    os.system("./build_glslang_spirv_tools.sh")
    os.chdir(WORK_PATH)

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
        
    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")
    

    os.chdir(WORK_PATH)


    shutil.rmtree(SPIRV_CROSS_BUILD_PATH)

def build_SPIRV_Cross_for_ios():


    os.chdir(SPIRV_CROSS_PATH)
    #os.system("./checkout_glslang_spirv_tools.sh")
    #os.system("./build_glslang_spirv_tools.sh")
    os.chdir(WORK_PATH)

    CMAKE_PATH = "cmake"


    if not os.path.exists(SPIRV_CROSS_BUILD_PATH):
            os.makedirs(SPIRV_CROSS_BUILD_PATH)
            
    os.chdir(SPIRV_CROSS_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' -G Xcode ' +
      " -DCMAKE_INSTALL_PREFIX=" + SPIRV_CROSS_INSTALL_PATH +
      " -DCMAKE_TOOLCHAIN_FILE=" + WORK_PATH + "/ios-cmake/ios.toolchain.cmake" +
      " -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=L8DNJAKB7A" +
      " -DSPIRV_CROSS_SKIP_INSTALL=TRUE" +
      " -DPLATFORM=OS64COMBINED" +
      " " + SPIRV_CROSS_PATH)


    if not os.path.exists(SPIRV_CROSS_INSTALL_PATH): 
        os.makedirs(SPIRV_CROSS_INSTALL_PATH)
        
    os.system(CMAKE_PATH + " --build ./ --target \
    spirv-cross-core \
    spirv-cross-glsl \
    spirv-cross-hlsl \
    spirv-cross-msl \
    spirv-cross-reflect \
    spirv-cross-c \
    spirv-cross-cpp \
    spirv-cross-util \
    --config Release")


    output_list = [
      "Release-iphoneos"

    ]


    if os.path.isdir(SPIRV_CROSS_INSTALL_PATH + "/lib-arm64"):
      shutil.rmtree(SPIRV_CROSS_INSTALL_PATH + "/lib-arm64")

    os.mkdir(SPIRV_CROSS_INSTALL_PATH + "/lib-arm64")
    
    for i in range(len(output_list)):
      dir_full_path = SPIRV_CROSS_BUILD_PATH + "/" + output_list[i]
      dir_files=os.listdir(dir_full_path)
      for file in dir_files:
        file_path=os.path.join(dir_full_path,file)  
        if os.path.isfile(file_path):          
          shutil.move(file_path, SPIRV_CROSS_INSTALL_PATH + "/lib-arm64/" + file)


    os.chdir(WORK_PATH)


    shutil.rmtree(SPIRV_CROSS_BUILD_PATH)





#build_SPIRV_Cross_for_windows()
build_SPIRV_Cross_for_ios()
