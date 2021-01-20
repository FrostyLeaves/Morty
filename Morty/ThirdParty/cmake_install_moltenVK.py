import os
import sys
import shutil


WORK_PATH = os.getcwd()
MOLTENVK_PATH = WORK_PATH + "/MoltenVK"
MOLTENVK_INSTALL_PATH = WORK_PATH + "/installs/MoltenVK"

MOLTENVK_BUILD_PATH = WORK_PATH + "/MoltenVK-Build"

def build_MoltenVK_for_windows():

    os.chdir(MOLTENVK_PATH)

    os.system("./fetchDependencies --macos")

    os.chdir(WORK_PATH)




build_MoltenVK_for_windows()

