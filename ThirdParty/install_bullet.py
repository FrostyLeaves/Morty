import os
import sys
import shutil

WORK_PATH = os.getcwd() + "/ThirdParty"
BULLET_PATH = WORK_PATH + "/Bullet"
BULLET_INSTALL_PATH = WORK_PATH + "/installs/Bullet"
BULLET_BUILD_PATH = WORK_PATH + "/Bullet-Build"

if not os.path.exists(BULLET_INSTALL_PATH):
    os.makedirs(BULLET_INSTALL_PATH) 

def find_cmake(path):
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        return file_path
    return None

def build_for_windows():

    CMAKE_PATH = "cmake"

    if not os.path.exists(BULLET_BUILD_PATH):
            os.makedirs(BULLET_BUILD_PATH)
            
    os.chdir(BULLET_BUILD_PATH)

    os.system(CMAKE_PATH +
      ' -G "Visual Studio 16 2019" '+
      ' -DCMAKE_INSTALL_PREFIX=' + BULLET_INSTALL_PATH +
      ' -DINSTALL_LIBS:BOOL=ON' +
      ' -DCMAKE_CXX_FLAGS_DEBUG=/MTd' +
      ' -DCMAKE_CXX_FLAGS_RELEASE=/MT' +
      ' ' + BULLET_PATH
    )

    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")

    os.chdir(WORK_PATH)

    shutil.rmtree(BULLET_BUILD_PATH)

def build_for_macos():

    CMAKE_PATH = "cmake"

    if not os.path.exists(BULLET_BUILD_PATH):
            os.makedirs(BULLET_BUILD_PATH)
            
    os.chdir(BULLET_BUILD_PATH)

    os.system(CMAKE_PATH +
      ' -G "XCode" '+
      ' -DCMAKE_INSTALL_PREFIX=' + BULLET_INSTALL_PATH +
      ' -DINSTALL_LIBS:BOOL=ON' +
      ' ' + BULLET_PATH
    )

    os.system(CMAKE_PATH + " --build ./ --target install --config Debug")
    os.system(CMAKE_PATH + " --build ./ --target install --config Release")

    os.chdir(WORK_PATH)

    shutil.rmtree(BULLET_BUILD_PATH)

def build_for_ios():
    pass
