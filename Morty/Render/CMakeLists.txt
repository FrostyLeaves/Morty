# CMakeList.txt: Render 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

set(RENDER_CODE_DIRECTORIES)

############################################################
# Render
############################################################
file(GLOB RENDER_MODULE_FILES ./*.h ./*.cpp)
source_group("Module" FILES ${RENDER_MODULE_FILES})

file(GLOB RENDER_BASIC_FILES Basic/*.h Basic/*.cpp)
source_group("Basic" FILES ${RENDER_BASIC_FILES})

file(GLOB RENDER_COMPONENT_FILES Component/*.h Component/*.cpp)
source_group("Component" FILES ${RENDER_COMPONENT_FILES})

file(GLOB RENDER_MATERIAL_FILES Material/*.h Material/*.cpp)
source_group("Material" FILES ${RENDER_MATERIAL_FILES})

file(GLOB RENDER_MERGE_INSTANCING_FILES MergeInstancing/*.h MergeInstancing/*.cpp)
source_group("MergeInstancing" FILES ${RENDER_MERGE_INSTANCING_FILES})

file(GLOB RENDER_MODEL_FILES Model/*.h Model/*.cpp)
source_group("Model" FILES ${RENDER_MODEL_FILES})

file(GLOB RENDER_RENDER_FILES Render/*.h Render/*.cpp)
source_group("Render" FILES ${RENDER_RENDER_FILES})

file(GLOB RENDER_RENDER_VULKAN_FILES Render/Vulkan/*.h Render/Vulkan/*.cpp)
source_group("Render\\Vulkan" FILES ${RENDER_RENDER_VULKAN_FILES})

file(GLOB RENDER_RENDER_GRAPH_FILES RenderGraph/*.h RenderGraph/*.cpp)
source_group("RenderGraph" FILES ${RENDER_RENDER_GRAPH_FILES})

file(GLOB RENDER_RENDER_PROGRAM_FILES RenderProgram/*.h RenderProgram/*.cpp)
source_group("RenderProgram" FILES ${RENDER_RENDER_PROGRAM_FILES})

file(GLOB RENDER_RESOURCE_FILES Resource/*.h Resource/*.cpp)
source_group("Resource" FILES ${RENDER_RESOURCE_FILES})

file(GLOB RENDER_SUBSYSTEM_FILES SubSystem/*.h SubSystem/*.cpp)
source_group("SubSystem" FILES ${RENDER_SUBSYSTEM_FILES})

file(GLOB RENDER_SYSTEM_FILES System/*.h System/*.cpp)
source_group("System" FILES ${RENDER_SYSTEM_FILES})

file(GLOB RENDER_VIEW_FILES View/*.h View/*.cpp)
source_group("View" FILES ${RENDER_VIEW_FILES})

file(GLOB RENDER_FLATBUFFER_FILES Flatbuffer/*.h Flatbuffer/*.cpp )
source_group("Flatbuffer" FILES ${RENDER_FLATBUFFER_FILES})

############################################################
# add library
############################################################
add_library(Render STATIC
    ${RENDER_MODULE_FILES}
    ${RENDER_BASIC_FILES}
    ${RENDER_COMPONENT_FILES}
    ${RENDER_FLATBUFFER_FILES}
    ${RENDER_RENDER_FILES}
    ${RENDER_RENDER_VULKAN_FILES}
    ${RENDER_RENDER_GRAPH_FILES}
    ${RENDER_RENDER_PROGRAM_FILES}
    ${RENDER_MATERIAL_FILES}
    ${RENDER_MERGE_INSTANCING_FILES}
    ${RENDER_MODEL_FILES}
    ${RENDER_RESOURCE_FILES}
    ${RENDER_SUBSYSTEM_FILES}
    ${RENDER_SYSTEM_FILES}
    ${RENDER_VIEW_FILES}

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