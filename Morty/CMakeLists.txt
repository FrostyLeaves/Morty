# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("Morty")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


set(MORTY_BUILD_TARGET_RANGE "WIN" "MACOS" "IOS")
set(MORTY_BUILD_TARGET "WIN" CACHE STRING "param for platform")
set_property(CACHE MORTY_BUILD_TARGET PROPERTY STRINGS ${MORTY_BUILD_TARGET_RANGE})

option(MORTY_BUILD_WITH_PHYSICS "Build With Physics" False)

if(NOT MORTY_BUILD_TARGET IN_LIST MORTY_BUILD_TARGET_RANGE)
    message(FATAL_ERROR "MORTY_BUILD_TARGET must be one of ${MORTY_BUILD_TARGET_RANGE}" " MORTY_BUILD_TARGET == " ${MORTY_BUILD_TARGET})
endif()

# 包含子项目。
add_subdirectory ("Morty")
add_subdirectory ("Editor")
add_subdirectory ("Demo")
