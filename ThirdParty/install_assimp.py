import ThirdParty
import os
import sys
import shutil
import codecs
import chardet

WORK_PATH = os.getcwd() + "/ThirdParty"
ASSIMP_PATH = WORK_PATH + "/assimp"
ASSIMP_INSTALL_PATH = WORK_PATH + "/installs/assimp"
ASSIMP_BUILD_PATH = WORK_PATH + "/assimp-Build"

if not os.path.exists(ASSIMP_INSTALL_PATH):
    os.makedirs(ASSIMP_INSTALL_PATH) 

def find_cmake(path):
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        return file_path
    return None

def build_for_android():


    ANDROID_SDK_PATH="Android/Sdk"
    NDK_PATH="Android/Sdk/ndk/21.3.6528147"
    CMAKE_TOOLCHAIN_FILE = NDK_PATH + "/build/cmake/android.toolchain.cmake"


    CMAKE_DIR = find_cmake(ANDROID_SDK_PATH + "/cmake")
    CMAKE_PATH = CMAKE_DIR + "/bin/cmake"
    NINJA_PATH = CMAKE_DIR + "/bin/ninja"


    ARM_LIST = [
        {"arch" : "arm", "abi" : "armeabi-v7a"},
        {"arch" : "arm64", "abi" : "arm64-v8a"}
    ]

    for arm_type in ARM_LIST:
            
        temp_dir = ASSIMP_BUILD_PATH + arm_type["abi"]
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir)

        os.chdir(temp_dir)

        NDK_TOOLCHAIN = temp_dir + "/llvm-" + arm_type["arch"]
        make_standalone_toolchain = NDK_PATH + "/build/tools/make_standalone_toolchain.py --arch=" + arm_type["arch"] + " --stl=libc++ --api=24 --install-dir=" + NDK_TOOLCHAIN
        os.system("py " + make_standalone_toolchain)


        os.system(CMAKE_PATH +
        " -GNinja" +
        " -DCMAKE_TOOLCHAIN_FILE=" + CMAKE_TOOLCHAIN_FILE +
        " -DASSIMP_ANDROID_JNIIOSYSTEM=ON" +
        " -DANDROID_NDK=" + NDK_PATH +
        " -DCMAKE_MAKE_PROGRAM=" + NINJA_PATH +
        " -DCMAKE_BUILD_TYPE=Release" +
        " -DANDROID_ABI=" + arm_type["abi"] +
        " -DANDROID_NATIVE_API_LEVEL=24" +
        " -DANDROID_FORCE_ARM_BUILD=TRUE" +
        " -DCMAKE_INSTALL_PREFIX=install" +
        " -DANDROID_STL=c++_shared" +
        " -DCMAKE_CXX_FLAGS=-Wno-c++11-narrowing" +
        " -DANDROID_TOOLCHAIN=clang" +
        " -DASSIMP_BUILD_TESTS=OFF" +
        " " + ASSIMP_PATH
        )

        os.system(CMAKE_PATH +
        " --build ./"#BUILD_PATH
        )

        output_path = ASSIMP_INSTALL_PATH + "/" + arm_type["abi"]
        if not os.path.exists(output_path):
            os.makedirs(output_path)

        for root, dirs, files in os.walk(temp_dir + "/bin", topdown=False):
            for file_name in files:
                shutil.move(os.path.join(root, file_name), os.path.join(output_path, file_name))

        os.chdir(WORK_PATH)

        shutil.rmtree(temp_dir)

def procBOM(strPath, bAdd):
    
    f = open(strPath, "rb")
    fcontent = f.read()
    f.close()

    codeType = chardet.detect(fcontent)["encoding"]
    if codeType.lower().find('utf-8') == -1 and codeType.lower().find('ascii') == -1 :
        return
    
    if fcontent[:3] != codecs.BOM_UTF8:
        fcontent = codecs.BOM_UTF8 + fcontent
        
        f = open(strPath, "wb+")
        f.write(fcontent)
        f.close()

def build_for_windows( cmake_generator ):

    for root, _, files in os.walk(ASSIMP_PATH + "/code/AssetLib/AMF", topdown=False):
        for name in files:
            file_full_path = os.path.join(root, name)
            procBOM(file_full_path, True)

    CMAKE_PATH = "cmake"

    if not os.path.exists(ASSIMP_BUILD_PATH):
            os.makedirs(ASSIMP_BUILD_PATH)
            
    os.chdir(ASSIMP_BUILD_PATH)

    os.system(CMAKE_PATH +
      ' -G "' + cmake_generator + '" '+
      ' -DCMAKE_INSTALL_PREFIX=' + ASSIMP_INSTALL_PATH +
      ' -DCMAKE_CXX_FLAGS_DEBUG=/MTd' +
      ' -DCMAKE_CXX_FLAGS_RELEASE=/MT' +
      ' ' + ASSIMP_PATH
    )

    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")

    os.chdir(WORK_PATH)

    shutil.rmtree(ASSIMP_BUILD_PATH)


def build_for_macos():

    for root, _, files in os.walk(ASSIMP_PATH + "/code/AssetLib/AMF", topdown=False):
        for name in files:
            file_full_path = os.path.join(root, name)
            procBOM(file_full_path, True)

    CMAKE_PATH = "cmake"

    if not os.path.exists(ASSIMP_BUILD_PATH):
            os.makedirs(ASSIMP_BUILD_PATH)
            
    os.chdir(ASSIMP_BUILD_PATH)

    os.system(CMAKE_PATH +
      ' -G "XCode" '+
      ' -DCMAKE_INSTALL_PREFIX=' + ASSIMP_INSTALL_PATH +
      ' ' + ASSIMP_PATH
    )

    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")

    os.chdir(WORK_PATH)

    shutil.rmtree(ASSIMP_BUILD_PATH)

def build_for_ios():
    os.chdir(ASSIMP_PATH + "/port/iOS")

    os.system("./build.sh")

    os.chdir(WORK_PATH)
