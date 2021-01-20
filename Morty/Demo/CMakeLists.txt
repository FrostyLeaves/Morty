cmake_minimum_required (VERSION 3.8)

project(Demo)



add_executable(Demo Main.cpp)

target_link_libraries(Demo Morty)
target_link_libraries(Demo Editor)

if(WIN32)
add_custom_command(TARGET Demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/Morty/Morty.dll"
        $<TARGET_FILE_DIR:Demo>)

add_custom_command(TARGET Demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/assimp/bin/assimp-vc142-mtd.dll"
        $<TARGET_FILE_DIR:Demo>)

add_custom_command(TARGET Demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/assimp/bin/assimp-vc142-mt.dll"
        $<TARGET_FILE_DIR:Demo>)

add_custom_command(TARGET Demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/bin/SDL2d.dll"
        $<TARGET_FILE_DIR:Demo>)
        
add_custom_command(TARGET Demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SDL/bin/SDL2.dll"
        $<TARGET_FILE_DIR:Demo>)
endif()