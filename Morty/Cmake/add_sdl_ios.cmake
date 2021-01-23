
############################################################
# SDL
############################################################

add_library(SDL2::SDL2 UNKNOWN IMPORTED)

set_target_properties(SDL2::SDL2
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/SDL/include"
                                IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/lib/Release-iphoneos/libSDL2.a"
)

set(SDL2_FOUND True)
