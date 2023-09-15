# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.20)


if(MORTY_BUILD_TARGET STREQUAL "ANDROID")
# There are backslashes instead of forward slashes in the path of the Android NDK.
	string(REPLACE "\\" "/" CMAKE_ANDROID_NDK ${CMAKE_ANDROID_NDK})
endif()

project ("Morty" CXX)

set(MORTY_BUILD_TARGET_RANGE "WIN" "MACOS" "IOS" "ANDROID")
set(MORTY_BUILD_TARGET "WIN" CACHE STRING "param for platform")
set_property(CACHE MORTY_BUILD_TARGET PROPERTY STRINGS ${MORTY_BUILD_TARGET_RANGE})

set(CMAKE_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../Cmake")
set(THIRD_PARTY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty")

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif(MSVC)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #Set this to ON if you want to use GNU++
set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} -O3 -Wall")

file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)

file(CREATE_LINK
	${CMAKE_SOURCE_DIR}/../Resource/Shader
	${CMAKE_SOURCE_DIR}/Shader
	SYMBOLIC
)

option(MORTY_BUILD_WITH_PHYSICS "Build With Physics" False)

if(NOT MORTY_BUILD_TARGET IN_LIST MORTY_BUILD_TARGET_RANGE)
    message(FATAL_ERROR "MORTY_BUILD_TARGET must be one of ${MORTY_BUILD_TARGET_RANGE}" " MORTY_BUILD_TARGET == " ${MORTY_BUILD_TARGET})
endif()

if(MORTY_BUILD_WITH_PHYSICS)
	message("Plugins/Physics")
	add_subdirectory ("Plugins/Physics")
endif()
add_subdirectory("Core")
add_subdirectory("Render")
add_subdirectory("Editor")

if(MORTY_BUILD_TARGET STREQUAL "WIN")
	add_subdirectory("Test")
	add_subdirectory("Demo")
elseif(MORTY_BUILD_TARGET STREQUAL "MACOS")
	add_subdirectory("Test")
	add_subdirectory("Demo")
elseif(MORTY_BUILD_TARGET STREQUAL "ANDROID")
	add_subdirectory("Android")
endif()