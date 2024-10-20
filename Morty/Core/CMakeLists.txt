# CMakeList.txt: Core 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required(VERSION 3.10)


############################################################
# add library
############################################################
file(GLOB_RECURSE CORE_All_FILES *.h *.cpp)
add_library(Core OBJECT
        ${CORE_All_FILES}
)


if (MORTY_BUILD_TARGET STREQUAL "WIN")
    target_compile_definitions(Core
            PRIVATE CORE_EXPORTS
            PUBLIC MORTY_WIN
            PUBLIC NOMINMAX
            PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
    #    message(FATAL_ERROR WIN)
endif ()

if (MORTY_BUILD_TARGET STREQUAL "MACOS")
    target_compile_definitions(Core
            PUBLIC MORTY_MACOS
            PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
    #    message(FATAL_ERROR MACOS)
endif ()

if (MORTY_BUILD_TARGET STREQUAL "IOS")
    target_compile_definitions(Core
            PUBLIC MORTY_IOS
    )
    #    message(FATAL_ERROR IOS)
endif ()

if (MORTY_BUILD_TARGET STREQUAL "LINUX")
    target_compile_definitions(Core
            PUBLIC MORTY_LINUX
            PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
    #    message(FATAL_ERROR LINUX)
endif ()


if (MORTY_BUILD_TARGET STREQUAL "ANDROID")
    target_compile_definitions(Core
            PUBLIC MORTY_ANDROID
            PUBLIC MORTY_RESOURCE_PATH="./assets/Resource"
    )
    #    message(FATAL_ERROR MACOS)
endif ()

target_compile_definitions(Core
        PUBLIC $<$<CONFIG:Debug>:MORTY_DEBUG>
)


#profiler
if (MORTY_PROFILER STREQUAL "ON")
    target_compile_definitions(Core
            PUBLIC MORTY_PROFILER=1
            PUBLIC TRACY_ENABLE
    )
endif ()

############################################################
# ThirdParty
############################################################

find_package(crossguid CONFIG REQUIRED)
if (crossguid_FOUND)
    target_link_libraries(Core PRIVATE crossguid)
else ()
    message(FATAL_ERROR "crossguid not found.")
endif ()

find_package(Eigen3 CONFIG REQUIRED)
if (Eigen3_FOUND)
    target_link_libraries(Core PRIVATE Eigen3::Eigen)
else ()
    message(FATAL_ERROR "Eigen3 not found.")
endif ()

find_package(flatbuffers CONFIG REQUIRED)
if (flatbuffers_FOUND)
    target_link_libraries(Core PUBLIC flatbuffers::flatbuffers)
else ()
    message(FATAL_ERROR "flatbuffers not found.")
endif ()

find_package(fmt CONFIG REQUIRED)
if (flatbuffers_FOUND)
    target_link_libraries(Core PUBLIC fmt::fmt)
else ()
    message(FATAL_ERROR "fmt not found.")
endif ()

find_package(tracy CONFIG REQUIRED)
if (tracy_FOUND)
    target_link_libraries(Core PRIVATE Tracy::TracyClient)
else ()
    message(FATAL_ERROR "tracy not found.")
endif ()

target_include_directories(Core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer
)

set_target_properties(Core PROPERTIES
        LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
        LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
        LINK_FLAGS "/ignore:4099"
)

generate_flatbuffers_header(Core)