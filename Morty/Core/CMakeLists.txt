# CMakeList.txt: Core 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

set(CORE_CODE_DIRECTORIES)

############################################################
# Core
############################################################
file(GLOB CORE_COMPONENT_FILES Component/*.h Component/*.cpp)
source_group("Component" FILES ${CORE_COMPONENT_FILES})

file(GLOB CORE_ENGINE_FILES Engine/*.h Engine/*.cpp)
source_group("Engine" FILES ${CORE_ENGINE_FILES})

file(GLOB CORE_INPUT_FILES Input/*.h Input/*.cpp)
source_group("Input" FILES ${CORE_INPUT_FILES})

file(GLOB CORE_MATH_FILES Math/*.h Math/*.cpp )
source_group("Math" FILES ${CORE_MATH_FILES})

file(GLOB CORE_MODULE_FILES Module/*.h Module/*.cpp )
source_group("Module" FILES ${CORE_MODULE_FILES})

file(GLOB CORE_OBJECT_FILES Object/*.h Object/*.cpp )
source_group("Object" FILES ${CORE_OBJECT_FILES})

file(GLOB CORE_POINTER_FILES Pointer/*.h Pointer/*.cpp )
source_group("Pointer" FILES ${CORE_POINTER_FILES})

file(GLOB CORE_RESOURCE_FILES Resource/*.h Resource/*.cpp )
source_group("Resource" FILES ${CORE_RESOURCE_FILES})

file(GLOB CORE_SCENE_FILES Scene/*.h Scene/*.cpp )
source_group("Scene" FILES ${CORE_SCENE_FILES})

file(GLOB EDITOR_SUBSYSTEM_FILES SubSystem/*.h SubSystem/*.cpp)
source_group("SubSystem" FILES ${EDITOR_SUBSYSTEM_FILES})

file(GLOB CORE_SYSTEM_FILES System/*.h System/*.cpp )
source_group("System" FILES ${CORE_SYSTEM_FILES})

file(GLOB CORE_TASK_GRAPH_FILES TaskGraph/*.h TaskGraph/*.cpp )
source_group("TaskGraph" FILES ${CORE_TASK_GRAPH_FILES})

file(GLOB CORE_THREAD_FILES Thread/*.h Thread/*.cpp )
source_group("Thread" FILES ${CORE_THREAD_FILES})

file(GLOB CORE_TYPE_FILES Type/*.h Type/*.cpp )
source_group("Type" FILES ${CORE_TYPE_FILES})

file(GLOB CORE_UTILITY_FILES Utility/*.h Utility/*.cpp )
source_group("Utility" FILES ${CORE_UTILITY_FILES})

file(GLOB CORE_VARIANT_FILES Variant/*.h Variant/*.cpp )
source_group("Variant" FILES ${CORE_VARIANT_FILES})

file(GLOB CORE_FLATBUFFER_FILES Flatbuffer/*.h Flatbuffer/*.cpp )
source_group("Flatbuffer" FILES ${CORE_FLATBUFFER_FILES})
############################################################
# add library
############################################################
add_library(Core STATIC 
    ${CORE_CORE_FILES}
    ${CORE_COMPONENT_FILES}
    ${CORE_ENGINE_FILES}
    ${CORE_INPUT_FILES}
    ${CORE_MATH_FILES}
    ${CORE_MODULE_FILES}
    ${CORE_OBJECT_FILES}
    ${CORE_POINTER_FILES}
    ${CORE_RESOURCE_FILES}
    ${CORE_SCENE_FILES}
    ${EDITOR_SUBSYSTEM_FILES}
    ${CORE_SYSTEM_FILES}
    ${CORE_TASK_GRAPH_FILES}
    ${CORE_THREAD_FILES}
    ${CORE_TYPE_FILES}
    ${CORE_UTILITY_FILES}
    ${CORE_VARIANT_FILES}
    ${CORE_FLATBUFFER_FILES}
)


############################################################
# header search path
############################################################
set(CORE_CODE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
        ${CORE_CODE_DIRECTORIES}
        #${CMAKE_CURRENT_SOURCE_DIR}/Component
        #${CMAKE_CURRENT_SOURCE_DIR}/Engine
        #${CMAKE_CURRENT_SOURCE_DIR}/Input
        #${CMAKE_CURRENT_SOURCE_DIR}/Math
        #${CMAKE_CURRENT_SOURCE_DIR}/Module
        #${CMAKE_CURRENT_SOURCE_DIR}/Scene
        #${CMAKE_CURRENT_SOURCE_DIR}/System
        #${CMAKE_CURRENT_SOURCE_DIR}/Object
        #${CMAKE_CURRENT_SOURCE_DIR}/Pointer
        #${CMAKE_CURRENT_SOURCE_DIR}/Resource
        #${CMAKE_CURRENT_SOURCE_DIR}/TaskGraph
        #${CMAKE_CURRENT_SOURCE_DIR}/Thread
        #${CMAKE_CURRENT_SOURCE_DIR}/Type
        #${CMAKE_CURRENT_SOURCE_DIR}/Utility
        #${CMAKE_CURRENT_SOURCE_DIR}/Variant
        ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer
)

############################################################
# ThirdParty
############################################################

if(MORTY_BUILD_TARGET STREQUAL "WIN")
    include(${CMAKE_SCRIPT_PATH}/add_vulkan.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_spirv.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_assimp.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_flatbuffer.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_crossguid.cmake)

    target_compile_definitions(Core
        PRIVATE CORE_EXPORTS
        PUBLIC MORTY_WIN
        PUBLIC NOMINMAX
        PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
#    message(FATAL_ERROR WIN)
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    include(${CMAKE_SCRIPT_PATH}/add_vulkan_macos.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_spirv.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_assimp.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_flatbuffer.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_crossguid.cmake)
    target_compile_definitions(Core
        PUBLIC MORTY_MACOS
    )
#    message(FATAL_ERROR MACOS)
endif()

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    include(${CMAKE_SCRIPT_PATH}/add_vulkan_ios.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_spirv_ios.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_assimp_ios.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_flatbuffer_ios.cmake)
    include(${CMAKE_SCRIPT_PATH}/add_crossguid_ios.cmake)
    target_compile_definitions(Core
        PUBLIC MORTY_IOS
    )
#    message(FATAL_ERROR IOS)
endif()


if(Vulkan_FOUND)
    target_link_libraries(Core PUBLIC Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found.")
endif()

if(assimp_FOUND)
    target_link_libraries(Core PRIVATE assimp::assimp)
else()
    message(FATAL_ERROR "Assimp not found.")
endif()

if(flatbuffers_FOUND)
    target_link_libraries(Core PUBLIC flatbuffers::flatbuffers)
else()
    message(FATAL_ERROR "flatbuffers not found.")
endif()

if(crossguid_FOUND)
    target_link_libraries(Core PUBLIC crossguid)
else()
    message(FATAL_ERROR "crossguid not found.")
endif()

if(SPIRV_CROSS_FOUND)
    target_include_directories(Core
        PUBLIC
            SPIRV_INCLUDE_PATH
    )

    target_link_directories(Core PRIVATE SPIRV_LIBRARY_PATH)
    target_link_libraries(Core PRIVATE SPIRV::ALL)
else()
    message(FATAL_ERROR "SPIRV_CROSS not found.")
endif()


target_include_directories(Core
    PUBLIC
        "${THIRD_PARTY_PATH}/MoltenVK/External/SPIRV-Cross"
)

############################################################
# Eigen
############################################################
target_include_directories(Core
    PUBLIC
        ${THIRD_PARTY_PATH}/eigen/Eigen
)

############################################################
# rapidjson
############################################################
target_include_directories(Core
    PUBLIC
        ${THIRD_PARTY_PATH}/rapidjson/include/rapidjson
)

set( CORE_CODE_DIRECTORIES
    ${CORE_CODE_DIRECTORIES}
    CACHE INTERNAL "Core Source Code Directories"
)

target_include_directories(Core
    PUBLIC 
        ${CORE_CODE_DIRECTORIES}
)

set_target_properties(Core PROPERTIES
         LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
         LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
         LINK_FLAGS "/ignore:4099"
)

