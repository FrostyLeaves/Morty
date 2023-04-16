# CMakeList.txt: Render 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

set(RENDER_CODE_DIRECTORIES)

############################################################
# Render
############################################################
file(GLOB_RECURSE RENDER_All_FILES *.h *.cpp)

############################################################
# add library
############################################################
add_library(Render STATIC
    ${RENDER_All_FILES}

    ${SPOT_FILES}
    ${STB_FILES}
)


############################################################
# header search path
############################################################
set(RENDER_CODE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
        ${RENDER_CODE_DIRECTORIES}
)

set( RENDER_CODE_DIRECTORIES
    ${RENDER_CODE_DIRECTORIES}
    CACHE INTERNAL "Render Source Code Directories"
)

target_include_directories(Render
    PUBLIC 
        ${RENDER_CODE_DIRECTORIES}
)

set_target_properties(Render PROPERTIES
         LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
         LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
         LINK_FLAGS "/ignore:4099"
)


if(MORTY_BUILD_TARGET STREQUAL "WIN")
    include(${CMAKE_SCRIPT_PATH}/add_spirv.cmake)
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    include(${CMAKE_SCRIPT_PATH}/add_spirv.cmake)
endif()

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    include(${CMAKE_SCRIPT_PATH}/add_spirv_ios.cmake)
endif()

find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    target_link_libraries(Render PUBLIC Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found.")
endif()

find_package(assimp CONFIG REQUIRED)
if(assimp_FOUND)
    target_link_libraries(Render PRIVATE assimp::assimp)
else()
    message(FATAL_ERROR "Assimp not found.")
endif()

if(SPIRV_CROSS_FOUND)
    target_link_libraries(Render PUBLIC SPIRV::ALL)
else()
    message(FATAL_ERROR "SPIRV_CROSS not found.")
endif()

find_package(directx-dxc CONFIG REQUIRED)
if(directx-dxc_FOUND)
    target_link_libraries(Render PRIVATE Microsoft::DirectXShaderCompiler)
else()
    message(FATAL_ERROR "directx-dxc not found.")
endif()

find_package(Stb REQUIRED)
if(Stb_FOUND)
    target_compile_definitions(Render PRIVATE STB_IMAGE_IMPLEMENTATION )
    target_include_directories(Render PRIVATE ${Stb_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Stb not found.")
endif()

target_link_libraries(Render PUBLIC Core)