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
        ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer
)

if(MORTY_BUILD_TARGET STREQUAL "WIN")
    target_compile_definitions(Core
        PRIVATE CORE_EXPORTS
        PUBLIC MORTY_WIN
        PUBLIC NOMINMAX
        PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
#    message(FATAL_ERROR WIN)
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    target_compile_definitions(Core
        PUBLIC MORTY_MACOS
    )
#    message(FATAL_ERROR MACOS)
endif()

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    target_compile_definitions(Core
        PUBLIC MORTY_IOS
    )
#    message(FATAL_ERROR IOS)
endif()


############################################################
# ThirdParty
############################################################

find_package(crossguid CONFIG REQUIRED)
if(crossguid_FOUND)
    target_link_libraries(Core PUBLIC crossguid)
else()
    message(FATAL_ERROR "crossguid not found.")
endif()

find_package(Eigen3 CONFIG REQUIRED)
if(Eigen3_FOUND)
    target_link_libraries(Core PUBLIC Eigen3::Eigen)
else()
    message(FATAL_ERROR "Eigen3 not found.")
endif()

find_package(flatbuffers CONFIG REQUIRED)
if(flatbuffers_FOUND)
    target_link_libraries(Core PUBLIC flatbuffers::flatbuffers)
else()
    message(FATAL_ERROR "flatbuffers not found.")
endif()

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

