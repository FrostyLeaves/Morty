cmake_minimum_required (VERSION 3.20)

############################################################
# Render
############################################################
file(GLOB_RECURSE EDITOR_All_FILES *.h *.cpp)

############################################################
# ThirdParty ImGuiFileDialog
############################################################
file(GLOB EDITOR_IMGUI_FILE_DIALOG_FILES ${THIRD_PARTY_PATH}/ImGuiFileDialog/*.h ${THIRD_PARTY_PATH}/ImGuiFileDialog/*.cpp )
if (NOT MSVC)
    set_property(SOURCE ${EDITOR_IMGUI_FILE_DIALOG_FILES} APPEND PROPERTY COMPILE_OPTIONS "-Wno-everything")
endif()

add_library(Editor OBJECT
        ${EDITOR_All_FILES}

        ${EDITOR_IMGUI_FILES}
        ${EDITOR_IMGUI_FILE_DIALOG_FILES}
)


target_link_libraries(Editor PUBLIC Core)
target_link_libraries(Editor PUBLIC Render)

if(MORTY_BUILD_WITH_PHYSICS)
target_link_libraries(Editor PUBLIC Physics)
endif()



############################################################
# header search path
############################################################
set(EDITOR_CODE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
        ${EDITOR_CODE_DIRECTORIES}
        ${THIRD_PARTY_PATH}/ImGuiFileDialog
)

set( EDITOR_CODE_DIRECTORIES
    ${EDITOR_CODE_DIRECTORIES}
    CACHE INTERNAL "Editor Source Code Directories"
)

target_include_directories(Editor
    PUBLIC 
        ${EDITOR_CODE_DIRECTORIES}
)

find_package(SDL2 CONFIG REQUIRED)
if(SDL2_FOUND)
    target_link_libraries(Editor PUBLIC SDL2::SDL2)
else()
    message(FATAL_ERROR "SDL2 not found.")
endif()

find_package(imgui CONFIG REQUIRED)
if(imgui_FOUND)
    target_link_libraries(Editor PUBLIC imgui::imgui)
    #target_compile_definitions(Editor
    #    PUBLIC IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/Utility/imgui_include.h"
    #)
else()
    message(FATAL_ERROR "imgui not found.")
endif()

find_package(imguizmo CONFIG REQUIRED)
if(imguizmo_FOUND)
    target_link_libraries(Editor PUBLIC imguizmo::imguizmo)
else()
    message(FATAL_ERROR "imguizmo not found.")
endif()

find_package(Stb REQUIRED)
if(Stb_FOUND)
    target_compile_definitions(Editor PRIVATE STB_IMAGE_IMPLEMENTATION )
    target_compile_definitions(Editor PRIVATE STB_IMAGE_WRITE_IMPLEMENTATION )
    target_include_directories(Editor PRIVATE ${Stb_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Stb not found.")
endif()
