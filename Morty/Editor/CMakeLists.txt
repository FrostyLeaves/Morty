cmake_minimum_required (VERSION 3.10)

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

file(GLOB EDITOR_TEST_FILES Test/*.h Test/*.cpp)
source_group("Test" FILES ${EDITOR_TEST_FILES})

file(GLOB EDITOR_UTILITY_FILES Utility/*.h Utility/*.cpp)
source_group("Utility" FILES ${EDITOR_UTILITY_FILES})

file(GLOB EDITOR_WIDGET_FILES Widget/*.h Widget/*.cpp)
source_group("Widget" FILES ${EDITOR_WIDGET_FILES})

############################################################
# ImGUI
############################################################
file(GLOB EDITOR_IMGUI_FILE_DIALOG_FILES ${THIRD_PARTY_PATH}/ImGuiFileDialog/ImGuiFileDialog/*.h ${THIRD_PARTY_PATH}/ImGuiFileDialog/ImGuiFileDialog/*.cpp )
source_group("ImGUI\\FileDialog" FILES ${EDITOR_IMGUI_FILE_DIALOG_FILES})

add_executable(Editor 
        ${EDITOR_COMPONENT_FILES}
        ${EDITOR_MAIN_FILES}
        ${EDITOR_MODULE_FILES}
        ${EDITOR_PROPERTY_FILES}
        ${EDITOR_RENDER_FILES}
        ${EDITOR_SYSTEM_FILES}
        ${EDITOR_TEST_FILES}
        ${EDITOR_UTILITY_FILES}
        ${EDITOR_WIDGET_FILES}
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
        ${THIRD_PARTY_PATH}/ImGuiFileDialog/ImGuiFileDialog
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
    target_compile_definitions(Editor
        PUBLIC IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/Utility/imgui_include.h"
    )
else()
    message(FATAL_ERROR "imgui not found.")
endif()

find_package(Stb REQUIRED)
if(Stb_FOUND)
    target_compile_definitions(Editor PRIVATE STB_IMAGE_IMPLEMENTATION )
    target_compile_definitions(Editor PRIVATE STB_IMAGE_WRITE_IMPLEMENTATION )
    target_include_directories(Editor PRIVATE ${Stb_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Stb not found.")
endif()