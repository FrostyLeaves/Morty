############################################################
# SDL
############################################################

if(WIN32)
    set(SDL2_DIR ${THIRD_PARTY_PATH}/installs/SDL/cmake)
else()
    set(SDL2_DIR ${THIRD_PARTY_PATH}/installs/SDL/lib/cmake/SDL2)
endif()

find_package(SDL2)
