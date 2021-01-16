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


add_library(Editor STATIC
	
	${EDITOR_FILES}
	${EDITOR_PROPERTY_FILES}

	${IMGUI_FILES}
	${IMGUI_FILE_DIALOG_FILES}

)

############################################################
# header search path
############################################################
set(EDITOR_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Property
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGUI
        ${CMAKE_CURRENT_SOURCE_DIR}/ImGUI/ImGuiFileDialog

        CACHE INTERNAL "Editor Source Code Directories"
)

target_include_directories(Editor
    PUBLIC 
        ${MORTY_CODE_DIRECTORIES}
        ${EDITOR_CODE_DIRECTORIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/dirent
)

############################################################
# Vulkan
############################################################
find_package(Vulkan)
if(Vulkan_FOUND)
    target_link_libraries(Editor PRIVATE Vulkan::Vulkan)
else(Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan library not found")
endif(Vulkan_FOUND)

############################################################
# Morty
############################################################

