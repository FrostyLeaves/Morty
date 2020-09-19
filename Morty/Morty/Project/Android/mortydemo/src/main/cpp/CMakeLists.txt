# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

SET(MORTY_SLN_PATH ${CMAKE_SOURCE_DIR}/../../../../../../../)
SET(MORTY_CODE_PATH ${CMAKE_SOURCE_DIR}/../../../../../../)
SET(NDK_PATH ${ANDROID_NDK})

SET(VULKAN_PATH $ENV{VK_SDK_PATH})

SET(MORTY_INCLUDE_DIR
        ${VULKAN_PATH}/Include
        ${MORTY_SLN_PATH}ThirdParty/spot
        ${MORTY_SLN_PATH}ThirdParty/rapidjson/include
        ${MORTY_SLN_PATH}Morty/Math
        ${MORTY_SLN_PATH}Morty/Utility
        ${MORTY_SLN_PATH}Morty/ResourceLoader
        ${MORTY_SLN_PATH}Morty/Organizer
        ${MORTY_SLN_PATH}Morty/Organizer/System
        ${MORTY_SLN_PATH}Morty/Organizer/Node
        ${MORTY_SLN_PATH}Morty/Core
        ${MORTY_SLN_PATH}Morty/Core/Engine
        ${MORTY_SLN_PATH}Morty/Core/RefCounter
        ${MORTY_SLN_PATH}Morty/Core/Log
        ${MORTY_SLN_PATH}Morty/Core/Renderer
        ${MORTY_SLN_PATH}Morty/Core/Renderer/DirectX11
        ${MORTY_SLN_PATH}Morty/Core/Renderer/Vulkan
        ${MORTY_SLN_PATH}Morty/Core/Input
        ${MORTY_SLN_PATH}Morty/Core/Painter
        ${MORTY_SLN_PATH}Morty/Core/Animation
        ${MORTY_SLN_PATH}Morty/Core/RenderProgram
        ${NDK_PATH}/sources/android/cpufeatures
)

add_library( # Sets the name of the library.
            MortyDemo

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
            demo_main.cpp

        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_include_directories(MortyDemo
        PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${MORTY_INCLUDE_DIR}
        )

# Morty
add_library(Morty SHARED IMPORTED)

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set_target_properties(Morty
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}Morty/Project/Android/app/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libMorty.so
)

target_link_libraries( # Specifies the target library.
        MortyDemo

                       # Links the target library to the log library
                       # included in the NDK.
        Morty
                       ${log-lib}
        android
        native_app_glue
        )