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
if(APPLE)
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/dirent/include
endif()
)

if(WIN32)
    target_compile_definitions(Editor
        PUBLIC MORTY_WIN
)

elseif(APPLE)
    target_compile_definitions(Editor
        PUBLIC MORTY_MACOS
)
endif()


if(WIN32)
############################################################
# Vulkan
############################################################
    find_package(Vulkan)
    if(Vulkan_FOUND)
        target_link_libraries(Editor PRIVATE Vulkan::Vulkan)
    else(Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan library not found")
    endif(Vulkan_FOUND)

elseif(APPLE)
############################################################
# MoltenVK
############################################################
target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK/include
)

target_link_directories(Editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK
)

add_library(MoltenVK UNKNOWN IMPORTED)

set_target_properties(MoltenVK
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK/include"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Debug/MoltenVK/dylib/macOS/libMoltenVK.dylib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Release/MoltenVK/dylib/macOS/libMoltenVK.dylib"
)


target_link_libraries(Editor PRIVATE MoltenVK)

endif()

############################################################
# Morty
############################################################

############################################################
# SDL
############################################################
target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/include/SDL2
)
if(WIN32)
    set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/cmake)
else()
    set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/lib/cmake/SDL2)
endif()

find_package(SDL2)

target_link_libraries(Editor PRIVATE SDL2::SDL2)