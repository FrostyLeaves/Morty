# CMakeList.txt: Render 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

############################################################
# Render
############################################################
file(GLOB_RECURSE RENDER_All_FILES *.h *.cpp)

############################################################
# add library
############################################################
add_library(Render OBJECT
    ${RENDER_All_FILES}

    ${SPOT_FILES}
)


############################################################
# header search path
############################################################

target_include_directories(Render
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer
)

set_target_properties(Render PROPERTIES
         LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
         LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
         LINK_FLAGS "/ignore:4099"
)

find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    target_link_libraries(Render PUBLIC Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found.")
endif()


find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)

if(NOT spirv_cross_glsl_FOUND)
    message(FATAL_ERROR "spirv_cross_glsl not found.")
elseif(NOT spirv_cross_hlsl_FOUND)
    message(FATAL_ERROR "spirv_cross_hlsl not found.")
elseif(NOT spirv_cross_msl_FOUND)
    message(FATAL_ERROR "spirv_cross_msl not found.")
elseif(NOT spirv_cross_cpp_FOUND)
    message(FATAL_ERROR "spirv_cross_cpp not found.")
elseif(NOT spirv_cross_reflect_FOUND)
    message(FATAL_ERROR "spirv_cross_reflect not found.")
elseif(NOT spirv_cross_util_FOUND)
    message(FATAL_ERROR "spirv_cross_util not found.")
else()
    target_link_libraries(Render PRIVATE spirv-cross-core)
    target_link_libraries(Render PRIVATE spirv-cross-glsl)
    target_link_libraries(Render PRIVATE spirv-cross-hlsl)
    target_link_libraries(Render PRIVATE spirv-cross-msl)
    target_link_libraries(Render PRIVATE spirv-cross-cpp)
    target_link_libraries(Render PRIVATE spirv-cross-reflect)
    target_link_libraries(Render PRIVATE spirv-cross-util)
endif()


find_package(assimp CONFIG REQUIRED)
if(assimp_FOUND)
    target_link_libraries(Render PRIVATE assimp::assimp)
else()
    message(FATAL_ERROR "Assimp not found.")
endif()

if(MORTY_BUILD_TARGET STREQUAL "WIN")

    find_package(directx-dxc CONFIG REQUIRED)
    if(directx-dxc_FOUND)
        target_link_libraries(Render PRIVATE Microsoft::DirectXShaderCompiler)
    else()
        message(FATAL_ERROR "directx-dxc not found.")
    endif()
endif()

if(MORTY_SHADER_COMPILER STREQUAL "dxc")

    add_definitions(-DMORTY_SHADER_COMPILER_DXC)

endif()

# fix for macos :https://github.com/microsoft/vcpkg/issues/27121
find_package(Threads)
find_package(glslang CONFIG REQUIRED)
if(glslang_FOUND)
    add_definitions(-DENABLE_HLSL)
    target_link_libraries(Render PRIVATE glslang::OSDependent glslang::glslang glslang::MachineIndependent glslang::GenericCodeGen glslang::OGLCompiler glslang::SPVRemapper glslang::SPIRV glslang::HLSL)
else()
    message(FATAL_ERROR "glslang not found.")
endif()

find_package(Stb REQUIRED)
if(Stb_FOUND)
    target_compile_definitions(Render PRIVATE STB_IMAGE_IMPLEMENTATION )
    target_include_directories(Render PRIVATE ${Stb_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Stb not found.")
endif()

target_link_libraries(Render PUBLIC Core)

generate_flatbuffers_header(Render)
generate_reflection(Render)