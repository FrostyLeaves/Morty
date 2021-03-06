import os
import sys
import shutil

WORK_PATH = os.getcwd()
SDL_PATH = WORK_PATH + "/SDL"
SDL_INSTALL_PATH = WORK_PATH + "/installs/SDL"
SDL_BUILD_PATH = WORK_PATH + "/SDL-Build"

if not os.path.exists(SDL_INSTALL_PATH):
    os.makedirs(SDL_INSTALL_PATH) 

def find_cmake(path):
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        return file_path
    return None

def build_for_windows():

    CMAKE_PATH = "cmake"

    if not os.path.exists(SDL_BUILD_PATH):
            os.makedirs(SDL_BUILD_PATH)
            
    os.chdir(SDL_BUILD_PATH)

    os.system(CMAKE_PATH +
      ' --G "Visual Studio 16 Win64" '+
      ' -DCMAKE_INSTALL_PREFIX=' + SDL_INSTALL_PATH +
      ' ' + SDL_PATH
    )

    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")

    os.chdir(WORK_PATH)

    shutil.rmtree(SDL_BUILD_PATH)

def build_for_ios():

    if not os.path.exists(SDL_BUILD_PATH):
            os.makedirs(SDL_BUILD_PATH)
            
    os.chdir(SDL_PATH + "/Xcode/SDL")

    os.system('xcodebuild' +
    ' -scheme "Static Library-iOS"' +
    ' SYMROOT="' + SDL_INSTALL_PATH + '/lib"' +
    ' -configuration Release'
    )

    os.chdir(WORK_PATH)

    shutil.rmtree(SDL_BUILD_PATH)

