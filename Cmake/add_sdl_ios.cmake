
############################################################
# SDL
############################################################

add_library(SDL2::SDL2 UNKNOWN IMPORTED)

set_target_properties(SDL2::SDL2
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${THIRD_PARTY_PATH}/SDL/include"
                                IMPORTED_LOCATION "${THIRD_PARTY_PATH}/installs/SDL/lib/Release-iphoneos/libSDL2.a"
)

set(SDL2_FOUND True)
