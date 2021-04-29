import install_assimp
import install_glslang
import install_moltenVK
import install_sdl
import install_spirv_cross
import install_bullet

import sys
import getopt



if __name__ == '__main__':

    platform = None
    
    try:
        opts, _ = getopt.getopt(sys.argv[1:], 'p:', ["platform="])
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
        else:
            print("Unknow param: " + opt)
    pass


    if platform == "WIN":
        install_assimp.build_for_windows()
        install_sdl.build_for_windows()
        install_spirv_cross.build_for_windows()
        install_glslang.build_for_windows()
        install_bullet.build_for_windows()
    elif platform == "MACOS":
        install_assimp.build_for_windows()
        install_sdl.build_for_windows()
        install_spirv_cross.build_for_windows()
        install_glslang.build_for_windows()
        install_moltenVK.build_for_macos()
        install_bullet.build_for_macos()
    elif platform == "IOS":
        install_assimp.build_for_ios()
        install_sdl.build_for_ios()
        install_spirv_cross.build_for_ios()
        install_glslang.build_for_ios()
        install_moltenVK.build_for_ios()
        install_bullet.build_for_ios()
    

