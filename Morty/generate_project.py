import os
import sys
import shutil


WORK_PATH = os.getcwd()
MORTY_PATH = WORK_PATH

MORTY_BUILD_PATH = WORK_PATH + "/build"

def build_for_macos():

    CMAKE_PATH = "cmake"

    if not os.path.exists(MORTY_BUILD_PATH):
            os.makedirs(MORTY_BUILD_PATH)
            
    os.chdir(MORTY_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' -G Xcode ' +
      " -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=L8DNJAKB7A" +
      " -DMORTY_BUILD_TARGET=MACOS" +
      " " + MORTY_PATH)

    os.chdir(WORK_PATH)


def build_for_ios():

    CMAKE_PATH = "cmake"

    if not os.path.exists(MORTY_BUILD_PATH):
            os.makedirs(MORTY_BUILD_PATH)
            
    os.chdir(MORTY_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' -G Xcode ' +
      " -DCMAKE_TOOLCHAIN_FILE=" + WORK_PATH + "/ThirdParty/ios-cmake/ios.toolchain.cmake" +
      " -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=L8DNJAKB7A" +
      " -DMORTY_BUILD_TARGET=IOS" +
      " -DPLATFORM=OS64COMBINED" +
      " " + MORTY_PATH)

    os.chdir(WORK_PATH)

def build_for_windows():

    CMAKE_PATH = "cmake"

    if not os.path.exists(MORTY_BUILD_PATH):
            os.makedirs(MORTY_BUILD_PATH)
            
    os.chdir(MORTY_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' -G "Visual Studio 16 2019" ' +
      " -DMORTY_BUILD_TARGET=WIN" +
      " " + MORTY_PATH)

    os.chdir(WORK_PATH)

build_for_windows()