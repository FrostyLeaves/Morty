import cmake
import os
import sys
import shutil


WORK_PATH = os.getcwd()
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

def build_assimp_for_android():


    ANDROID_SDK_PATH="C:/Users/null.ptr/AppData/Local/Android/Sdk"
    NDK_PATH="C:/Users/null.ptr/AppData/Local/Android/Sdk/ndk/21.3.6528147"
    CMAKE_TOOLCHAIN_FILE = NDK_PATH + "/build/cmake/android.toolchain.cmake"


    CMAKE_DIR = find_cmake(ANDROID_SDK_PATH + "/cmake")
    CMAKE_PATH = CMAKE_DIR + "/bin/cmake"
    NINJA_PATH = CMAKE_DIR + "/bin/ninja"


    ARM_LIST = [
        {"arch" : "arm", "abi" : "armeabi-v7a"},
        {"arch" : "arm64", "abi" : "arm64-v8a"}
    ]

    for arm_type in ARM_LIST:
            
        temp_dir = BUILD_PATH + arm_type["abi"]
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

def build_assimp_for_windows():

    CMAKE_PATH = "cmake"

    if not os.path.exists(ASSIMP_BUILD_PATH):
            os.makedirs(ASSIMP_BUILD_PATH)
            
    os.chdir(ASSIMP_BUILD_PATH)

    os.system(CMAKE_PATH +
      ' --G "Visual Studio 16 Win64" '+
      ' -DCMAKE_INSTALL_PREFIX=' + ASSIMP_INSTALL_PATH +
      ' ' + ASSIMP_PATH
    )

    os.system(CMAKE_PATH + " --build ./ --target INSTALL --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target INSTALL --config Release")

    os.chdir(WORK_PATH)

    shutil.rmtree(ASSIMP_BUILD_PATH)

#build_assimp_for_android()
build_assimp_for_windows()

