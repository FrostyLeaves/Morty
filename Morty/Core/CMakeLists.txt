# CMakeList.txt: Core 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

set(CORE_CODE_DIRECTORIES)

############################################################
# Core
############################################################
file(GLOB_RECURSE CORE_All_FILES *.h *.cpp)

############################################################
# add library
############################################################
add_library(Core STATIC 
    ${CORE_All_FILES}
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

