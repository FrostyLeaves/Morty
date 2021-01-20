# CMakeList.txt: Morty 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project(Morty)


#target_compile_definitions(Morty, PRIVATE 
#    
#)
 
set(MORTY_CODE_DIRECTORIES)

############################################################
# Math
############################################################
file(GLOB MORTY_MATH_FILES Math/*.h Math/*.cpp)
source_group("Math" FILES ${MORTY_MATH_FILES})


############################################################
# Core
############################################################
file(GLOB MORTY_CORE_FILES Core/*.h Core/*.cpp)
source_group("Core" FILES ${MORTY_CORE_FILES})

file(GLOB MORTY_ENGINE_FILES Core/Engine/*.h Core/Engine/*.cpp)
source_group("Core/Engine" FILES ${MORTY_ENGINE_FILES})

file(GLOB MORTY_ANIMATION_FILES Core/Animation/*.h Core/Animation/*.cpp)
source_group("Core/Animation" FILES ${MORTY_ANIMATION_FILES})

file(GLOB MORTY_INPUT_FILES Core/Input/*.h Core/Input/*.cpp )
source_group("Core/Input" FILES ${MORTY_INPUT_FILES})

file(GLOB MORTY_LOG_FILES Core/Log/*.h Core/Log/*.cpp )
source_group("Core/Log" FILES ${MORTY_LOG_FILES})

file(GLOB MORTY_PAINTER_FILES Core/Painter/*.h Core/Painter/*.cpp )
source_group("Core/Painter" FILES ${MORTY_PAINTER_FILES})

file(GLOB MORTY_REF_COUNTER_FILES Core/RefCounter/*.h Core/RefCounter/*.cpp )
source_group("Core/RefCounter" FILES ${MORTY_REF_COUNTER_FILES})

file(GLOB MORTY_RENDERER_FILES Core/Renderer/*.h Core/Renderer/*.cpp )
source_group("Core/Renderer" FILES ${MORTY_RENDERER_FILES})

file(GLOB MORTY_SHADER_FILES Core/Renderer/Shader/*.h Core/Renderer/Shader/*.cpp )
source_group("Core/Renderer/Shader" FILES ${MORTY_SHADER_FILES})

file(GLOB MORTY_VULKAN_PROGRAM_FILES Core/Renderer/Vulkan/*.h Core/Renderer/Vulkan/*.cpp )
source_group("Core/Renderer/Vulkan" FILES ${MORTY_VULKAN_PROGRAM_FILES})

file(GLOB MORTY_DX11_PROGRAM_FILES Core/Renderer/DirectX11/*.h Core/Renderer/DirectX11/*.cpp )
source_group("Core/Renderer/DirectX11" FILES ${MORTY_DX11_PROGRAM_FILES})

file(GLOB MORTY_RENDER_PROGRAM_FILES Core/RenderProgram/*.h Core/RenderProgram/*.cpp )
source_group("Core/RenderProgram" FILES ${MORTY_RENDER_PROGRAM_FILES})


############################################################
# Organizer
############################################################
file(GLOB MORTY_ORGANIZER_FILES Organizer/*.h Organizer/*.cpp )
source_group("Organizer" FILES ${MORTY_ORGANIZER_FILES})

file(GLOB MORTY_NODE_FILES Organizer/Node/*.h Organizer/Node/*.cpp )
source_group("Organizer/Node" FILES ${MORTY_NODE_FILES})

file(GLOB MORTY_NODE_LIGHT_FILES Organizer/Node/Light/*.h Organizer/Node/Light/*.cpp )
source_group("Organizer/Node/Light" FILES ${MORTY_NODE_LIGHT_FILES})

file(GLOB MORTY_NODE_MODEL_FILES Organizer/Node/Model/*.h Organizer/Node/Model/*.cpp )
source_group("Organizer/Node/Model" FILES ${MORTY_NODE_MODEL_FILES})


############################################################
# Resource Loader
############################################################
file(GLOB MORTY_RES_LOADER_FILES ResourceLoader/*.h ResourceLoader/*.cpp )
source_group("ResourceLoader" FILES ${MORTY_RES_LOADER_FILES})

file(GLOB MORTY_RES_MATERIAL_FILES ResourceLoader/Material/*.h ResourceLoader/Material/*.cpp )
source_group("ResourceLoader/Material" FILES ${MORTY_RES_MATERIAL_FILES})

file(GLOB MORTY_RES_MODEL_FILES ResourceLoader/Model/*.h ResourceLoader/Model/*.cpp )
source_group("ResourceLoader/Model" FILES ${MORTY_RES_MODEL_FILES})

file(GLOB MORTY_RES_NODE_FILES ResourceLoader/Node/*.h ResourceLoader/Node/*.cpp )
source_group("ResourceLoader/Node" FILES ${MORTY_RES_NODE_FILES})

file(GLOB MORTY_RES_RENDER_PASS_FILES ResourceLoader/RenderPass/*.h ResourceLoader/RenderPass/*.cpp )
source_group("ResourceLoader/RenderPass" FILES ${MORTY_RES_RENDER_PASS_FILES})

file(GLOB MORTY_RES_SHADER_FILES ResourceLoader/Shader/*.h ResourceLoader/Shader/*.cpp )
source_group("ResourceLoader/Shader" FILES ${MORTY_RES_SHADER_FILES})

file(GLOB MORTY_RES_TEXTURE_FILES ResourceLoader/Texture/*.h ResourceLoader/Texture/*.cpp )
source_group("ResourceLoader/Texture" FILES ${MORTY_RES_TEXTURE_FILES})


############################################################
# Utility
############################################################
file(GLOB MORTY_UTILITY_FILES Utility/*.h Utility/*.cpp )
source_group("Utility" FILES ${MORTY_UTILITY_FILES})

file(GLOB MORTY_JSON_FILES Utility/Json/*.h Utility/Json/*.cpp )
source_group("Utility/Json" FILES ${MORTY_JSON_FILES})

file(GLOB MORTY_TIMER_FILES Utility/Timer/*.h Utility/Timer/*.cpp )
source_group("Utility/Timer" FILES ${MORTY_TIMER_FILES})

file(GLOB MORTY_TYPE_FILES Utility/Type/*.h Utility/Type/*.cpp )
source_group("Utility/Type" FILES ${MORTY_TYPE_FILES})


############################################################
# spot
############################################################
file(GLOB SPOT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spot/*.hpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spot/*.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spot/*.c
)
source_group("spot" FILES ${SPOT_FILES})
set(MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/spot)


############################################################
# add library
############################################################
add_library(Morty SHARED 
    ${MORTY_MATH_FILES}

    ${MORTY_CORE_FILES}
    ${MORTY_ENGINE_FILES}
    ${MORTY_ANIMATION_FILES}
    ${MORTY_INPUT_FILES}
    ${MORTY_LOG_FILES}
    ${MORTY_PAINTER_FILES}
    ${MORTY_REF_COUNTER_FILES}
    ${MORTY_RENDERER_FILES}
    ${MORTY_SHADER_FILES}
    ${MORTY_VULKAN_PROGRAM_FILES}
    ${MORTY_DX11_PROGRAM_FILES}
    ${MORTY_RENDER_PROGRAM_FILES}

    ${MORTY_ORGANIZER_FILES}
    ${MORTY_NODE_FILES}
    ${MORTY_NODE_LIGHT_FILES}
    ${MORTY_NODE_MODEL_FILES}

    ${MORTY_RES_LOADER_FILES}
    ${MORTY_RES_MATERIAL_FILES}
    ${MORTY_RES_MODEL_FILES}
    ${MORTY_RES_NODE_FILES}
    ${MORTY_RES_RENDER_PASS_FILES}
    ${MORTY_RES_SHADER_FILES}
    ${MORTY_RES_TEXTURE_FILES}

    ${MORTY_UTILITY_FILES}
    ${MORTY_JSON_FILES}
    ${MORTY_TIMER_FILES}
    ${MORTY_TYPE_FILES}

    ${SPOT_FILES}
)


############################################################
# header search path
############################################################
set(MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/Math

        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Animation
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Input
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Log
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Painter
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/RefCounter
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Renderer
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Renderer/Shader
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Renderer/Vulkan
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/RenderProgram

        ${CMAKE_CURRENT_SOURCE_DIR}/Organizer
        ${CMAKE_CURRENT_SOURCE_DIR}/Organizer/Node
        ${CMAKE_CURRENT_SOURCE_DIR}/Organizer/Node/Light
        ${CMAKE_CURRENT_SOURCE_DIR}/Organizer/Node/Model

        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader
        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader/Material
        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader/Model
        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader/Node
        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader/RenderPass
        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader/Shader
        ${CMAKE_CURRENT_SOURCE_DIR}/ResourceLoader/Texture

        ${CMAKE_CURRENT_SOURCE_DIR}/Utility
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility/Json
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility/Timer
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility/Type
)

############################################################
# ThirdParty
############################################################


if(WIN32)
    target_compile_definitions(Morty
        PRIVATE MORTY_EXPORTS
        PUBLIC MORTY_WIN
)

elseif(APPLE)
    target_compile_definitions(Morty
        PUBLIC MORTY_MACOS
)
endif()


if(WIN32)
############################################################
# Vulkan
############################################################
    find_package(Vulkan)
    if(Vulkan_FOUND)
        set(MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES} ${Vulkan_INCLUDE_DIRS})
        target_link_libraries(Morty PRIVATE Vulkan::Vulkan)
    else(Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan library not found")
    endif(Vulkan_FOUND)

elseif(APPLE)
############################################################
# MoltenVK
############################################################
target_include_directories(Morty
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK/include
)

target_link_directories(Morty
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK
)

add_library(MoltenVK UNKNOWN IMPORTED)

set_target_properties(MoltenVK
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK/include"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Debug/MoltenVK/dylib/macOS/libMoltenVK.dylib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Release/MoltenVK/dylib/macOS/libMoltenVK.dylib"
)


target_link_libraries(Morty PRIVATE MoltenVK)

endif()

############################################################
# SPIRV-Cross 
############################################################
set(spirv_cross_core_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/share/spirv_cross_core/cmake)
set(spirv_cross_glsl_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/share/spirv_cross_glsl/cmake)
set(spirv_cross_cpp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/share/spirv_cross_cpp/cmake)

find_package(spirv_cross_core)
find_package(spirv_cross_glsl)
find_package(spirv_cross_cpp)

set( MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/include/spirv_cross
)

target_link_directories(Morty
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/lib
)

target_link_libraries(Morty PRIVATE spirv-cross-core)
target_link_libraries(Morty PRIVATE spirv-cross-glsl)
target_link_libraries(Morty PRIVATE spirv-cross-cpp)


############################################################
# glslang
############################################################
if(NOT TARGET GLSLang) # this if is required for subsequent runs of CMake

    set(GLSLang_Include
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include/glslang/SPIRV"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include/glslang/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include/glslang/Public"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include"
    )

if(WIN32)
    set(PPRS "")
    set(PPOS_D "d.lib")
    set(PPOS_R ".lib")
elseif(APPLE)
    set(PPRS "lib")
    set(PPOS_D "d.a")
    set(PPOS_R ".a")
endif()

    
    add_library(GLSLang::GLSLang UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GLSLang
                        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}glslang${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}glslang${PPOS_R}"
    )

    add_library(GLSLang::SPIRV UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPIRV
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPIRV${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPIRV${PPOS_R}"
    )

    add_library(GLSLang::GenericCodeGen UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GenericCodeGen
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}GenericCodeGen${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}GenericCodeGen${PPOS_R}"
    )

    add_library(GLSLang::HLSL UNKNOWN IMPORTED)

    set_target_properties(GLSLang::HLSL
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}HLSL${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}HLSL${PPOS_R}"
    )

    add_library(GLSLang::MachineIndependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::MachineIndependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}MachineIndependent${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}MachineIndependent${PPOS_R}"
    )
    
    add_library(GLSLang::OGLCompiler UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OGLCompiler
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OGLCompiler${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OGLCompiler${PPOS_R}"
    )
    
    add_library(GLSLang::OSDependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OSDependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OSDependent${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OSDependent${PPOS_R}"
    )
    
    add_library(GLSLang::SPVRemapper UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPVRemapper
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLang_Include}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPVRemapper${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPVRemapper${PPOS_R}"
    )

    add_library(GLSLang::ALL INTERFACE IMPORTED)
    set_property(TARGET GLSLang::ALL PROPERTY INTERFACE_LINK_LIBRARIES
                    GLSLang::GLSLang
                    GLSLang::SPIRV
                    GLSLang::GenericCodeGen
                    GLSLang::HLSL
                    GLSLang::MachineIndependent
                    GLSLang::OGLCompiler
                    GLSLang::OSDependent
                    GLSLang::SPVRemapper)

    target_link_libraries(Morty PRIVATE GLSLang::ALL)

endif()


############################################################
# Eigen
############################################################
target_include_directories(Morty
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/eigen/Eigen
)


############################################################
# assimp
############################################################
target_include_directories(Morty
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/assimp/include
)
set(assimp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/assimp/lib/cmake/assimp-5.0)
find_package(assimp)

target_link_libraries(Morty PRIVATE assimp::assimp)


############################################################
# rapidjson
############################################################
target_include_directories(Morty
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/rapidjson/include/rapidjson
)

set( MORTY_CODE_DIRECTORIES
    ${MORTY_CODE_DIRECTORIES}
    CACHE INTERNAL "Morty Source Code Directories"
)

target_include_directories(Morty
    PUBLIC 
        ${MORTY_CODE_DIRECTORIES}
)

set_target_properties(Morty PROPERTIES
         LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
         LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
)