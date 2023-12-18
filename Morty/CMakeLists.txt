# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.20)

project ("Morty" CXX)


if(MORTY_BUILD_TARGET STREQUAL "ANDROID")
# There are backslashes instead of forward slashes in the path of the Android NDK.
	string(REPLACE "\\" "/" CMAKE_ANDROID_NDK ${CMAKE_ANDROID_NDK})
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2 -ggdb -fno-limit-debug-info")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target x86_64-w64-mingw32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target x86_64-w64-mingw32")
endif()


if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif(MSVC)

if (MORTY_TEST_COVERAGE)
	include(CTest)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage -fprofile-generate")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage -fprofile-generate")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -fprofile-generate")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} $ENV{CXXFLAGS} -O0")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} $ENV{CXXFLAGS} -O0")
else()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} $ENV{CXXFLAGS} -O3")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} $ENV{CXXFLAGS} -O0")
endif(MORTY_TEST_COVERAGE)


set(MORTY_BUILD_TARGET_RANGE "WIN" "LINUX" "MACOS" "IOS" "ANDROID")
set(MORTY_BUILD_TARGET "WIN" CACHE STRING "param for platform")
set(MORTY_BUILD_LIB_ONLY_PLATFORM "IOS" "ANDROID")
set_property(CACHE MORTY_BUILD_TARGET PROPERTY STRINGS ${MORTY_BUILD_TARGET_RANGE})

set(CMAKE_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../Cmake")
set(THIRD_PARTY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty")


set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #Set this to ON if you want to use GNU++
set(CMAKE_CXX_STANDARD 17)


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

if(NOT MORTY_BUILD_TARGET IN_LIST MORTY_BUILD_LIB_ONLY_PLATFORM)
	add_subdirectory("Test")
	add_subdirectory("Demo")
endif()

if(MORTY_BUILD_TARGET STREQUAL "ANDROID")
	add_subdirectory("Android")
endif()