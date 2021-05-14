# CMakeList.txt: Physics 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project(Physics)

set(PHYSICS_CODE_DIRECTORIES)

############################################################
# Files
############################################################
file(GLOB PHYSICS_PLUGIN_FILES *.h *.cpp)
source_group("Plugin" FILES ${PHYSICS_PLUGIN_FILES})


file(GLOB PHYSICS_COMPONENT_FILES Component/*.h Component/*.cpp)
source_group("Component" FILES ${PHYSICS_COMPONENT_FILES})

############################################################
# add library
############################################################
add_library(Physics STATIC 
    ${PHYSICS_PLUGIN_FILES}

    ${PHYSICS_COMPONENT_FILES}
)


############################################################
# header search path
############################################################
set(PHYSICS_CODE_DIRECTORIES ${PHYSICS_CODE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Component
)

############################################################
# ThirdParty
############################################################


if(PHYSICS_BUILD_TARGET STREQUAL "WIN")
    target_compile_definitions(Physics
        PRIVATE PHYSICS_EXPORTS
        PUBLIC PHYSICS_WIN
    )
endif()

if(PHYSICS_BUILD_TARGET STREQUAL "MACOS")
    target_compile_definitions(Physics
        PUBLIC PHYSICS_MACOS
    )
#    message(FATAL_ERROR MACOS)
endif()

if(PHYSICS_BUILD_TARGET STREQUAL "IOS")
    target_compile_definitions(Physics
        PUBLIC PHYSICS_IOS
    )
#    message(FATAL_ERROR IOS)
endif()


############################################################
# Physics
############################################################

include(add_bullet.cmake)

if(BULLET_FOUND)
    target_link_libraries(Physics PRIVATE BULLET::ALL)
else()
    message(FATAL_ERROR "Bullet not found.")
endif()


set( PHYSICS_CODE_DIRECTORIES
    ${PHYSICS_CODE_DIRECTORIES}
    CACHE INTERNAL "Physics Source Code Directories"
)

target_include_directories(Physics
    PUBLIC 
        ${MORTY_CODE_DIRECTORIES}
        ${PHYSICS_CODE_DIRECTORIES}
)

set_target_properties(Physics PROPERTIES
         LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
         LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
)

