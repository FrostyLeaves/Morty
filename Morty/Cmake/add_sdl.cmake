############################################################
# SDL
############################################################

if(WIN32)
    set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/cmake)
else()
    set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/lib/cmake/SDL2)
endif()

find_package(SDL2)
