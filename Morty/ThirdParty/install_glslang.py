import os
import sys
import shutil


WORK_PATH = os.getcwd()
GLSLANG_PATH = WORK_PATH + "/SPIRV-Cross/external/glslang"
GLSLANG_INSTALL_PATH = WORK_PATH + "/installs/glslang"

GLSLANG_BUILD_PATH = WORK_PATH + "/glslang-Build"

def build_for_windows():

    CMAKE_PATH = "cmake"


    if not os.path.exists(GLSLANG_BUILD_PATH):
            os.makedirs(GLSLANG_BUILD_PATH)
            
    os.chdir(GLSLANG_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' --G "Visual Studio 16 Win64" ' +
      " -DCMAKE_INSTALL_PREFIX=" + GLSLANG_INSTALL_PATH +
      ' -DCMAKE_CXX_FLAGS_DEBUG=/MTd' +
      ' -DCMAKE_CXX_FLAGS_RELEASE=/MT' +
      " -DCMAKE_DEBUG_POSTFIX=d" +
      " " + GLSLANG_PATH)


    if not os.path.exists(GLSLANG_INSTALL_PATH): 
        os.makedirs(GLSLANG_INSTALL_PATH)
        
    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")
    

    os.chdir(WORK_PATH)


    shutil.rmtree(GLSLANG_BUILD_PATH)


def build_for_macos():

    CMAKE_PATH = "cmake"


    if not os.path.exists(GLSLANG_BUILD_PATH):
            os.makedirs(GLSLANG_BUILD_PATH)
            
    os.chdir(GLSLANG_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' --G "XCode" ' +
      " -DCMAKE_INSTALL_PREFIX=" + GLSLANG_INSTALL_PATH +
      " -DCMAKE_DEBUG_POSTFIX=d" +
      " " + GLSLANG_PATH)


    if not os.path.exists(GLSLANG_INSTALL_PATH): 
        os.makedirs(GLSLANG_INSTALL_PATH)
        
    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")
    

    os.chdir(WORK_PATH)


    shutil.rmtree(GLSLANG_BUILD_PATH)

def build_for_ios():

    CMAKE_PATH = "cmake"


    if not os.path.exists(GLSLANG_BUILD_PATH):
            os.makedirs(GLSLANG_BUILD_PATH)
            
    os.chdir(GLSLANG_BUILD_PATH)


    os.system(CMAKE_PATH + 
      ' -G Xcode ' +
      " -DCMAKE_INSTALL_PREFIX=" + GLSLANG_INSTALL_PATH +
      " -DCMAKE_TOOLCHAIN_FILE=" + WORK_PATH + "/ios-cmake/ios.toolchain.cmake" +
      " -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=L8DNJAKB7A" +
      " -DPLATFORM=OS64COMBINED" +
      " -DSKIP_GLSLANG_INSTALL=True" +
      " -DENABLE_SPVREMAPPER=ON" +
      " " + GLSLANG_PATH)


    if not os.path.exists(GLSLANG_INSTALL_PATH): 
        os.makedirs(GLSLANG_INSTALL_PATH)
        
    os.system(CMAKE_PATH + " --build ./" + 
      " --target MachineIndependent " +
      " --target glslang" +
      " --target OSDependent" +
      " --target HLSL" +
      " --target OGLCompiler" +
      " --target SPIRV" +
      " --target SPVRemapper" +
      " --target glslang-default-resource-limits" +
      " --config Release"
    )
    
    os.chdir(WORK_PATH)

    output_list = [
      "hlsl/Release-iphoneos",
      "glslang/Release-iphoneos",
      "glslang/OSDependent/Unix/Release-iphoneos",
      "OGLCompilersDLL/Release-iphoneos",
      "SPIRV/Release-iphoneos",
      "StandAlone/Release-iphoneos",

    ]


    if os.path.isdir(GLSLANG_INSTALL_PATH + "/lib-arm64"):
      shutil.rmtree(GLSLANG_INSTALL_PATH + "/lib-arm64")

    os.mkdir(GLSLANG_INSTALL_PATH + "/lib-arm64")
    
    for i in range(len(output_list)):
      dir_full_path = GLSLANG_BUILD_PATH + "/" + output_list[i]
      dir_files=os.listdir(dir_full_path)
      for file in dir_files:
        file_path=os.path.join(dir_full_path,file)  
        if os.path.isfile(file_path):          
          shutil.move(file_path, GLSLANG_INSTALL_PATH + "/lib-arm64/" + file)

    
    shutil.rmtree(GLSLANG_BUILD_PATH)

