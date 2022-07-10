cmake_minimum_required (VERSION 3.10)

set(DEMO_INNER_RESOURCE "")

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    set(DEMO_INNER_RESOURCE ${DEMO_INNER_RESOURCE}
        "iOS Launch Screen.storyboard"
    )
endif()

add_executable(Demo Demo.cpp
        ${DEMO_INNER_RESOURCE})

target_link_libraries(Demo Core)
#target_link_libraries(Demo Render)

if(MORTY_BUILD_WITH_PHYSICS)
target_link_libraries(Demo Physics)
endif()

if(MORTY_BUILD_TARGET STREQUAL "WIN")

#    add_custom_command(TARGET Demo POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${CMAKE_BINARY_DIR}/Morty/Morty.dll"
#            $<TARGET_FILE_DIR:Demo>)

    add_custom_command(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/assimp/bin/assimp-vc142-mtd.dll"
            $<TARGET_FILE_DIR:Demo>)

    add_custom_command(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/assimp/bin/assimp-vc142-mt.dll"
            $<TARGET_FILE_DIR:Demo>)

    add_custom_command(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/SDL/bin/SDL2d.dll"
            $<TARGET_FILE_DIR:Demo>)
            
    add_custom_command(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${THIRD_PARTY_PATH}/installs/SDL/bin/SDL2.dll"
            $<TARGET_FILE_DIR:Demo>)

endif()


if(MORTY_BUILD_TARGET STREQUAL "IOS")

    set(APP_BUNDLE_IDENTIFIER me.doubleye.mortydemo)

    set(MACOSX_BUNDLE_INFO_STRING ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${APP_BUNDLE_IDENTIFIER})

    target_link_libraries(Demo "-framework Metal")
    target_link_libraries(Demo "-framework AudioToolbox")
    target_link_libraries(Demo "-framework AVFoundation")
    target_link_libraries(Demo "-framework CoreAudio")
    target_link_libraries(Demo "-framework CoreGraphics")
    target_link_libraries(Demo "-framework CoreHaptics")
    target_link_libraries(Demo "-framework CoreMotion")
    target_link_libraries(Demo "-framework Foundation")
    target_link_libraries(Demo "-framework GameController")
    target_link_libraries(Demo "-framework OpenGLES")
    target_link_libraries(Demo "-framework QuartzCore")
    target_link_libraries(Demo "-framework UIKit")

    set_target_properties(Demo
        PROPERTIES
            MACOSX_BUNDLE YES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/info.plist"
            XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
            RESOURCE "${DEMO_INNER_RESOURCE}"
            LINK_FLAGS "/ignore:4099"
    )

    set(DEMO_RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../Resource)

    target_sources(Demo PUBLIC ${DEMO_RESOURCE_FILES})

    set_source_files_properties(${DEMO_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

endif()