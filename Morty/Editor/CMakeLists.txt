cmake_minimum_required (VERSION 3.8)

project(Editor)

############################################################
# Editor
############################################################
file(GLOB EDITOR_FILES Editor/*.h Editor/*.cpp )
source_group("Editor" FILES ${EDITOR_FILES})

file(GLOB EDITOR_PROPERTY_FILES Editor/Property/*.h Editor/Property/*.cpp )
source_group("Editor/Property" FILES ${EDITOR_PROPERTY_FILES})


############################################################
# ImGUI
############################################################
file(GLOB IMGUI_FILES ImGUI/*.h ImGUI/*.cpp )
source_group("ImGUI" FILES ${IMGUI_FILES})

file(GLOB IMGUI_FILE_DIALOG_FILES ImGUI/ImGuiFileDialog/*.h ImGUI/ImGuiFileDialog/*.cpp )
source_group("ImGUI/FileDialog" FILES ${IMGUI_FILE_DIALOG_FILES})

file(GLOB IMGUI_NODE_EDITOR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui-node-editor/*.h ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui-node-editor/*.cpp )
source_group("ImGUI/NodeEditor" FILES ${IMGUI_NODE_EDITOR_FILES})



add_library(Editor STATIC
	
	${EDITOR_FILES}
	${EDITOR_PROPERTY_FILES}

	${IMGUI_FILES}
	${IMGUI_FILE_DIALOG_FILES}
    ${IMGUI_NODE_EDITOR_FILES}
)

############################################################
# header search path
############################################################
set(EDITOR_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Property
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGUI
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGUI/ImGuiFileDialog
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui-node-editor

        CACHE INTERNAL "Editor Source Code Directories"
)

target_include_directories(Editor
    PUBLIC 
        ${MORTY_CODE_DIRECTORIES}
        ${EDITOR_CODE_DIRECTORIES}
if(APPLE)
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/dirent/include
endif()
)



if(MORTY_BUILD_TARGET STREQUAL "WIN")
    include(../Cmake/add_vulkan.cmake)
    include(../Cmake/add_sdl.cmake)

    target_compile_definitions(Editor
        PUBLIC MORTY_WIN
    )
endif()

if(MORTY_BUILD_TARGET STREQUAL "MACOS")
    include(../Cmake/add_vulkan_macos.cmake)
    include(../Cmake/add_sdl.cmake)
    target_compile_definitions(Editor
        PUBLIC MORTY_MACOS
    )
#    message(FATAL_ERROR MACOS)
endif()

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    include(../Cmake/add_vulkan_ios.cmake)
    include(../Cmake/add_sdl_ios.cmake)
    target_compile_definitions(Editor
        PUBLIC MORTY_IOS
    )
endif()



if(Vulkan_FOUND)
    target_link_libraries(Editor PRIVATE Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found.")
endif()

if(SDL2_FOUND)
    target_link_libraries(Editor PUBLIC SDL2::SDL2)
else()
    message(FATAL_ERROR "SDL2 not found.")
endif()