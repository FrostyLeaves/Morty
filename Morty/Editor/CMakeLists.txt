cmake_minimum_required (VERSION 3.8)


############################################################
# Render
############################################################

file(GLOB EDITOR_COMPONENT_FILES Component/*.h Component/*.cpp)
source_group("Component" FILES ${EDITOR_COMPONENT_FILES})

file(GLOB EDITOR_MAIN_FILES Main/*.h Main/*.cpp)
source_group("Main" FILES ${EDITOR_MAIN_FILES})

file(GLOB EDITOR_MODULE_FILES Module/*.h Module/*.cpp)
source_group("Module" FILES ${EDITOR_MODULE_FILES})

file(GLOB EDITOR_PROPERTY_FILES Property/*.h Property/*.cpp)
source_group("Property" FILES ${EDITOR_PROPERTY_FILES})

file(GLOB EDITOR_RENDER_FILES Render/*.h Render/*.cpp)
source_group("Render" FILES ${EDITOR_RENDER_FILES})

file(GLOB EDITOR_SYSTEM_FILES System/*.h System/*.cpp)
source_group("System" FILES ${EDITOR_SYSTEM_FILES})

file(GLOB EDITOR_UTILITY_FILES Utility/*.h Utility/*.cpp)
source_group("Utility" FILES ${EDITOR_UTILITY_FILES})

file(GLOB EDITOR_WIDGET_FILES Widget/*.h Widget/*.cpp)
source_group("Widget" FILES ${EDITOR_WIDGET_FILES})

############################################################
# ImGUI
############################################################
file(GLOB EDITOR_IMGUI_FILES ${THIRD_PARTY_PATH}/imgui/*.h ${THIRD_PARTY_PATH}/imgui/*.cpp
    ${THIRD_PARTY_PATH}/imgui/backends/imgui_impl_vulkan.*
    ${THIRD_PARTY_PATH}/imgui/backends/imgui_impl_sdl.*
    ${THIRD_PARTY_PATH}/imgui/misc/cpp/imgui_stdlib.*)

source_group("ImGUI" FILES ${EDITOR_IMGUI_FILES})

file(GLOB EDITOR_IMGUI_FILE_DIALOG_FILES ${THIRD_PARTY_PATH}/ImGuiFileDialog/ImGuiFileDialog/*.h ${THIRD_PARTY_PATH}/ImGuiFileDialog/ImGuiFileDialog/*.cpp )
source_group("ImGUI\\FileDialog" FILES ${EDITOR_IMGUI_FILE_DIALOG_FILES})

file(GLOB EDITOR_IMGUI_NODE_EDITOR_FILES ${THIRD_PARTY_PATH}/imgui-node-editor/*.h ${THIRD_PARTY_PATH}/imgui-node-editor/*.cpp )
source_group("ImGUI\\NodeEditor" FILES ${EDITOR_IMGUI_NODE_EDITOR_FILES})

add_executable(Editor 
        ${EDITOR_COMPONENT_FILES}
        ${EDITOR_MAIN_FILES}
        ${EDITOR_MODULE_FILES}
        ${EDITOR_PROPERTY_FILES}
        ${EDITOR_RENDER_FILES}
        ${EDITOR_SYSTEM_FILES}
        ${EDITOR_UTILITY_FILES}
        ${EDITOR_WIDGET_FILES}
        ${EDITOR_IMGUI_FILES}
        ${EDITOR_IMGUI_FILE_DIALOG_FILES}
        ${EDITOR_IMGUI_NODE_EDITOR_FILES}
)


target_link_libraries(Editor PUBLIC Core)
target_link_libraries(Editor PUBLIC Render)

if(MORTY_BUILD_WITH_PHYSICS)
target_link_libraries(Editor PUBLIC Physics)
endif()



############################################################
# header search path
############################################################
set(EDITOR_CODE_DIRECTORIES ${RENDER_CODE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/Component
        ${CMAKE_CURRENT_SOURCE_DIR}/Main
        ${CMAKE_CURRENT_SOURCE_DIR}/Module
        ${CMAKE_CURRENT_SOURCE_DIR}/Property
        ${CMAKE_CURRENT_SOURCE_DIR}/Render
        ${CMAKE_CURRENT_SOURCE_DIR}/System
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility
        ${CMAKE_CURRENT_SOURCE_DIR}/Widget
        ${THIRD_PARTY_PATH}/imgui
        ${THIRD_PARTY_PATH}/imgui/backends
        ${THIRD_PARTY_PATH}/imgui/misc/cpp
        ${THIRD_PARTY_PATH}/ImGuiFileDialog/ImGuiFileDialog
        ${THIRD_PARTY_PATH}/imgui-node-editor
)

set( EDITOR_CODE_DIRECTORIES
    ${EDITOR_CODE_DIRECTORIES}
    CACHE INTERNAL "Editor Source Code Directories"
)

target_include_directories(Editor
    PUBLIC 
        ${EDITOR_CODE_DIRECTORIES}
)


if(MORTY_BUILD_TARGET STREQUAL "WIN")
    target_include_directories(Editor
        PUBLIC 
            ${THIRD_PARTY_PATH}/dirent/include
    )
endif()


if(MORTY_BUILD_TARGET STREQUAL "WIN")
    include(${CMAKE_SCRIPT_PATH}/add_sdl.cmake)
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    include(${CMAKE_SCRIPT_PATH}/add_sdl.cmake)
endif()


if(SDL2_FOUND)
    target_link_libraries(Editor PUBLIC SDL2::SDL2)
else()
    message(FATAL_ERROR "SDL2 not found.")
endif()


target_compile_definitions(Editor PUBLIC IMGUI_USER_CONFIG="imgui_include.h")



if(MORTY_BUILD_TARGET STREQUAL "WIN")

    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/assimp/bin/assimp-vc142-mtd.dll"
            $<TARGET_FILE_DIR:Editor>)

    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/assimp/bin/assimp-vc142-mt.dll"
            $<TARGET_FILE_DIR:Editor>)

    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/SDL/bin/SDL2d.dll"
            $<TARGET_FILE_DIR:Editor>)
            
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/SDL/bin/SDL2.dll"
            $<TARGET_FILE_DIR:Editor>)

endif()

