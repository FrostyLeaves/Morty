import os
import sys
import getopt

from ThirdParty import install_assimp
from ThirdParty import install_moltenVK
from ThirdParty import install_sdl
from ThirdParty import install_bullet


WORK_PATH = os.getcwd()
MORTY_PATH = WORK_PATH

MORTY_BUILD_PATH = WORK_PATH + "/Build"

def build_for_macos():

    CMAKE_PATH = "cmake"

    if not os.path.exists(MORTY_BUILD_PATH):
            os.makedirs(MORTY_BUILD_PATH)
            
    os.chdir(MORTY_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' -G Xcode ' +
      " -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=L8DNJAKB7A" +
      " -DMORTY_BUILD_TARGET=MACOS" +
      " " + MORTY_PATH + "/Morty")

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
      " -DPLATFORM=xxx" +
      " " + MORTY_PATH + "/Morty")

    os.chdir(WORK_PATH)

def build_for_windows():

    CMAKE_PATH = "cmake"

    if not os.path.exists(MORTY_BUILD_PATH):
            os.makedirs(MORTY_BUILD_PATH)
            
    os.chdir(MORTY_BUILD_PATH)

    os.system(CMAKE_PATH + 
      ' -G "Visual Studio 16 2019" ' +
      " -DMORTY_BUILD_TARGET=WIN" +
      " " + MORTY_PATH + "/Morty")

    os.chdir(WORK_PATH)


if __name__ == '__main__':

    platform = None
    target = None
    
    try:
        opts, _ = getopt.getopt(sys.argv[1:], 'p:t:', ["platform=", "target="])
    except getopt.GetoptError as e:
        print(e)
        exit(-1)

    for opt, value in opts:
        if opt in ("-p", "--platform"):
            if value in ("WIN", "MACOS", "IOS"):
                platform = value
            else:
                print("Unknow platform: " + value)
                print("Platform range: [WIN, MACOS, IOS]")
                exit(-1)
        elif opt in ("-t", "--target"):
            if value in ("Debug", "Release"):
                target = value
            else:
                print("Unknow target: " + value)
                print("Platform range: [Debug, Release]")
                exit(-1)
        else:
            print("Unknow param: " + opt)
    pass

    if platform == "WIN":
        #install_assimp.build_for_windows()
        #install_sdl.build_for_windows()
        #install_bullet.build_for_windows()
        build_for_windows()
    elif platform == "MACOS":
        install_assimp.build_for_macos()
        install_sdl.build_for_macos()
        install_moltenVK.build_for_macos()
        #install_bullet.build_for_macos()
        build_for_macos()
    elif platform == "IOS":
        install_assimp.build_for_ios()
        install_sdl.build_for_ios()
        install_moltenVK.build_for_ios()
        #install_bullet.build_for_ios()
        build_for_ios()
    else:
        print("use -p [WIN | MACOS | IOS] to select a platform.")
    

