cmake_minimum_required (VERSION 3.8)


############################################################
# Render
############################################################

set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/ImGUI)

file(GLOB EDITOR_MAIN_FILES Main/*.h Main/*.cpp)
source_group("Main" FILES ${EDITOR_MAIN_FILES})

file(GLOB EDITOR_PROPERTY_FILES Property/*.h Property/*.cpp)
source_group("Property" FILES ${EDITOR_PROPERTY_FILES})

file(GLOB EDITOR_RENDER_FILES Render/*.h Render/*.cpp)
source_group("Render" FILES ${EDITOR_RENDER_FILES})

file(GLOB EDITOR_UTILITY_FILES Utility/*.h Utility/*.cpp)
source_group("Utility" FILES ${EDITOR_UTILITY_FILES})

file(GLOB EDITOR_WIDGET_FILES Widget/*.h Widget/*.cpp)
source_group("Widget" FILES ${EDITOR_WIDGET_FILES})

############################################################
# ImGUI
############################################################
file(GLOB EDITOR_IMGUI_FILES ${IMGUI_PATH}/*.h ${IMGUI_PATH}/*.cpp)
source_group("ImGUI" FILES ${EDITOR_IMGUI_FILES})

file(GLOB EDITOR_IMGUI_FILE_DIALOG_FILES ${IMGUI_PATH}/ImGuiFileDialog/*.h ${IMGUI_PATH}/ImGuiFileDialog/*.cpp )
source_group("ImGUI\\FileDialog" FILES ${EDITOR_IMGUI_FILE_DIALOG_FILES})

file(GLOB EDITOR_IMGUI_NODE_EDITOR_FILES ${IMGUI_PATH}/../imgui-node-editor/*.h ${IMGUI_PATH}/../imgui-node-editor/*.cpp )
source_group("ImGUI\\NodeEditor" FILES ${EDITOR_IMGUI_NODE_EDITOR_FILES})

add_executable(Editor 
        ${EDITOR_MAIN_FILES}
        ${EDITOR_PROPERTY_FILES}
        ${EDITOR_RENDER_FILES}
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
        ${CMAKE_CURRENT_SOURCE_DIR}/Main
        ${CMAKE_CURRENT_SOURCE_DIR}/Property
        ${CMAKE_CURRENT_SOURCE_DIR}/Render
        ${CMAKE_CURRENT_SOURCE_DIR}/Utility
        ${CMAKE_CURRENT_SOURCE_DIR}/Widget
        ${IMGUI_PATH}
        ${IMGUI_PATH}/ImGuiFileDialog
        ${IMGUI_PATH}/../imgui-node-editor
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
            ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/dirent/include
    )
endif()


if(MORTY_BUILD_TARGET STREQUAL "WIN")
    include(../Cmake/add_sdl.cmake)
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    include(../Cmake/add_sdl.cmake)
endif()



if(SDL2_FOUND)
    target_link_libraries(Editor PUBLIC SDL2::SDL2)
else()
    message(FATAL_ERROR "SDL2 not found.")
endif()



if(MORTY_BUILD_TARGET STREQUAL "WIN")

    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/assimp/bin/assimp-vc142-mtd.dll"
            $<TARGET_FILE_DIR:Editor>)

    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/assimp/bin/assimp-vc142-mt.dll"
            $<TARGET_FILE_DIR:Editor>)

    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/bin/SDL2d.dll"
            $<TARGET_FILE_DIR:Editor>)
            
    add_custom_command(TARGET Editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/bin/SDL2.dll"
            $<TARGET_FILE_DIR:Editor>)

endif()

