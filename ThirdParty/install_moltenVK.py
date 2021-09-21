import os
import sys
import shutil


WORK_PATH = os.getcwd() + "/ThirdParty"
MOLTENVK_PATH = WORK_PATH + "/MoltenVK"
MOLTENVK_INSTALL_PATH = WORK_PATH + "/installs/MoltenVK"

MOLTENVK_BUILD_PATH = WORK_PATH + "/MoltenVK-Build"

def build_for_macos():

    os.chdir(MOLTENVK_PATH)

    os.system("./fetchDependencies --macos")

    os.system('xcodebuild' +
    ' -project MoltenVKPackaging.xcodeproj' +
    ' SYMROOT="' + MOLTENVK_INSTALL_PATH + '/lib"' +
    ' -scheme "MoltenVK-macOS"' +
    ' -configuration Release'
    )

    os.chdir(WORK_PATH)

def build_for_ios():

    os.chdir(MOLTENVK_PATH)

    os.system("./fetchDependencies --ios")

    os.system('xcodebuild' +
    ' -project MoltenVKPackaging.xcodeproj' +
    ' SYMROOT="' + MOLTENVK_INSTALL_PATH + '/lib-arm64"' +
    ' -scheme "MoltenVK-iOS"' +
    ' -configuration Release'
    )

    os.chdir(WORK_PATH)
