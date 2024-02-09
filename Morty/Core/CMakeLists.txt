# CMakeList.txt: Core 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)


############################################################
# add library
############################################################
file(GLOB_RECURSE CORE_All_FILES *.h *.cpp)
add_library(Core OBJECT
    ${CORE_All_FILES}
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
        PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
#    message(FATAL_ERROR MACOS)
endif()

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    target_compile_definitions(Core 
        PUBLIC MORTY_IOS
    )
#    message(FATAL_ERROR IOS)
endif()

if(MORTY_BUILD_TARGET STREQUAL "LINUX")
    target_compile_definitions(Core 
        PUBLIC MORTY_LINUX
        PUBLIC MORTY_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../Resource"
    )
#    message(FATAL_ERROR LINUX)
endif()



if(MORTY_BUILD_TARGET STREQUAL "ANDROID")
    target_compile_definitions(Core 
            PUBLIC MORTY_ANDROID
            PUBLIC MORTY_RESOURCE_PATH="./assets/Resource"
            )
    #    message(FATAL_ERROR MACOS)
endif()

target_compile_definitions(Core 
            PUBLIC $<$<CONFIG:Debug>:MORTY_DEBUG>
            )

############################################################
# ThirdParty
############################################################

find_package(crossguid CONFIG REQUIRED)
if(crossguid_FOUND)
    target_link_libraries(Core PRIVATE crossguid)
else()
    message(FATAL_ERROR "crossguid not found.")
endif()

find_package(Eigen3 CONFIG REQUIRED)
if(Eigen3_FOUND)
    target_link_libraries(Core PRIVATE Eigen3::Eigen)
else()
    message(FATAL_ERROR "Eigen3 not found.")
endif()

find_package(flatbuffers CONFIG REQUIRED)
if(flatbuffers_FOUND)
    target_link_libraries(Core  PUBLIC flatbuffers::flatbuffers)
else()
    message(FATAL_ERROR "flatbuffers not found.")
endif()

find_package(fmt CONFIG REQUIRED)
if(flatbuffers_FOUND)
    target_link_libraries(Core PUBLIC fmt::fmt)
else()
    message(FATAL_ERROR "fmt not found.")
endif()


target_include_directories(Core 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer
)

set_target_properties(Core  PROPERTIES
         LIBRARY_OUTPUT_PATH_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Debug
         LIBRARY_OUTPUT_PATH_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../out/build/Release
         LINK_FLAGS "/ignore:4099"
)


############################################################
# flatbuffers generator
############################################################
find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
function(generate_flatbuffers_header LIBRARY_NAME)

    file(GLOB_RECURSE ALL_FBS_PATH *.fbs)
    
    get_target_property(ALL_LIBRARY ${LIBRARY_NAME} INTERFACE_LINK_LIBRARIES )
    set(ALL_LIBRARY ${ALL_LIBRARY} ${LIBRARY_NAME})

    set(FLATBUFFERS_INCLUDE_DIR)

    foreach(PRE_LIBRARY ${ALL_LIBRARY})
        if (NOT ${PRE_LIBRARY} MATCHES "LINK_ONLY")
            get_target_property(LIB_INCLUDE_PATH_LIST ${PRE_LIBRARY} INTERFACE_INCLUDE_DIRECTORIES )
            foreach(PRE_INCLUDE_PATH ${LIB_INCLUDE_PATH_LIST})
                set(FLATBUFFERS_INCLUDE_DIR ${FLATBUFFERS_INCLUDE_DIR} "-I" ${PRE_INCLUDE_PATH})
            endforeach()
        endif()
    endforeach()

    message("[flatbuffers] ${FLATBUFFERS_FLATC_EXECUTABLE} ${FLATBUFFERS_INCLUDE_DIR} --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer/ ${SRC_FBS} --reflect-types --reflect-names ")
    foreach(SRC_FBS ${ALL_FBS_PATH})
        execute_process(
            COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE} ${FLATBUFFERS_INCLUDE_DIR} --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer/ ${SRC_FBS} --reflect-types --reflect-names
        )
    endforeach()
endfunction()

generate_flatbuffers_header(Core)
