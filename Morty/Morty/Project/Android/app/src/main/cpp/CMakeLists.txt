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
SET(NDK_PATH $NDK_ROOT)

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

file(GLOB_RECURSE MORTY_SOURCE_FILES ${MORTY_CODE_PATH}*.cpp
        )

include_directories(
             Morty
             ${MORTY_INCLUDE_DIR}
)

add_library( # Sets the name of the library.
            Morty

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ${MORTY_SOURCE_FILES}
             ${MORTY_SLN_PATH}ThirdParty/spot/spot.cpp
             ${MORTY_SLN_PATH}ThirdParty/spot/spotc.c
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

target_include_directories(Morty
        PRIVATE
        ${MORTY_SLN_PATH}ThirdParty/Assimp/include
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/include
        ${MORTY_SLN_PATH}ThirdParty/SPIRV/include
        ${MORTY_SLN_PATH}ThirdParty/Eigen
        ${MORTY_SLN_PATH}ThirdParty/glslang/include
        )


# assimp
add_library(assimp SHARED IMPORTED)

set_target_properties(assimp
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/Assimp/lib/clang/${ANDROID_ABI}/libassimp.so
)


#spirv_cross
add_library(spirv_cross_c STATIC IMPORTED)
add_library(spirv_cross_core STATIC IMPORTED)
add_library(spirv_cross_cpp STATIC IMPORTED)
add_library(spirv_cross_glsl STATIC IMPORTED)
add_library(spirv_cross_hlsl STATIC IMPORTED)
add_library(spirv_cross_msl STATIC IMPORTED)
add_library(spirv_cross_reflect STATIC IMPORTED)
add_library(spirv_cross_util STATIC IMPORTED)

set_target_properties(spirv_cross_c
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-c.a
        )
set_target_properties(spirv_cross_core
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-core.a
        )
set_target_properties(spirv_cross_cpp
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-cpp.a
        )
set_target_properties(spirv_cross_glsl
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-glsl.a
        )
set_target_properties(spirv_cross_hlsl
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-hlsl.a
        )
set_target_properties(spirv_cross_msl
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-msl.a
        )
set_target_properties(spirv_cross_reflect
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-reflect.a
        )
set_target_properties(spirv_cross_util
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SpirvCross/lib/clang/${ANDROID_ABI}/libspirv-cross-util.a
        )


#glslang
add_library(glslang STATIC IMPORTED)
add_library(glslang-default-resource-limits STATIC IMPORTED)
add_library(OGLCompiler STATIC IMPORTED)
add_library(OSDependent STATIC IMPORTED)
add_library(SPIRV STATIC IMPORTED)

set_target_properties(glslang
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/glslang/lib/clang/${ANDROID_ABI}/libglslang.a
        )

set_target_properties(glslang-default-resource-limits
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/glslang/lib/clang/${ANDROID_ABI}/libglslang-default-resource-limits.a
        )

set_target_properties(OGLCompiler
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/glslang/lib/clang/${ANDROID_ABI}/libOGLCompiler.a
        )

set_target_properties(OSDependent
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/glslang/lib/clang/${ANDROID_ABI}/libOSDependent.a
        )

set_target_properties(SPIRV
        PROPERTIES IMPORTED_LOCATION
        ${MORTY_SLN_PATH}ThirdParty/SPIRV/lib/clang/${ANDROID_ABI}/libSPIRV.a
        )

target_link_libraries( # Specifies the target library.
        Morty

                       # Links the target library to the log library
                       # included in the NDK.
        assimp
        spirv_cross_c
        spirv_cross_core
        spirv_cross_cpp
        spirv_cross_glsl
        spirv_cross_hlsl
        spirv_cross_msl
        spirv_cross_reflect
        spirv_cross_util
        glslang
        glslang-default-resource-limits
        OGLCompiler
        OSDependent
        SPIRV
                       ${log-lib}
        )