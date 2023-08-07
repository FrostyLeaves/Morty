cmake_minimum_required (VERSION 3.10)

file(GLOB_RECURSE MORTY_All_FILES *.h *.cpp)

add_library(morty SHARED
        ${MORTY_All_FILES})

target_link_libraries(morty Core)
target_link_libraries(morty Render)
target_link_libraries(morty Editor)

if(MORTY_BUILD_WITH_PHYSICS)
target_link_libraries(morty Physics)
endif()



find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        morty

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})