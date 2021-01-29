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
add_library(Morty STATIC 
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


if(MORTY_BUILD_TARGET STREQUAL "WIN")
    include(../Cmake/add_vulkan.cmake)
    include(../Cmake/add_spirv.cmake)
    include(../Cmake/add_assimp.cmake)

    target_compile_definitions(Morty
        PRIVATE MORTY_EXPORTS
        PUBLIC MORTY_WIN
    )
#    message(FATAL_ERROR WIN)
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    include(../Cmake/add_vulkan_macos.cmake)
    include(../Cmake/add_spirv.cmake)
    include(../Cmake/add_assimp.cmake)
    target_compile_definitions(Morty
        PUBLIC MORTY_MACOS
    )
#    message(FATAL_ERROR MACOS)
endif()

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    include(../Cmake/add_vulkan_ios.cmake)
    include(../Cmake/add_spirv_ios.cmake)
    include(../Cmake/add_assimp_ios.cmake)
    target_compile_definitions(Morty
        PUBLIC MORTY_IOS
    )
#    message(FATAL_ERROR IOS)
endif()



if(Vulkan_FOUND)
    target_link_libraries(Morty PUBLIC Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found.")
endif()

if(assimp_FOUND)
    target_link_libraries(Morty PRIVATE assimp::assimp)
else()
    message(FATAL_ERROR "Assimp not found.")
endif()

if(SPIRV_CROSS_FOUND)
    target_link_libraries(Morty PRIVATE SPIRV::ALL)
else()
    message(FATAL_ERROR "SPIRV_CROSS not found.")
endif()


target_include_directories(Morty
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/External/SPIRV-Cross"
)

############################################################
# Eigen
############################################################
target_include_directories(Morty
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/eigen/Eigen
)

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