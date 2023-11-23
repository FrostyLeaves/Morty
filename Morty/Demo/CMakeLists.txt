cmake_minimum_required (VERSION 3.10)

set(DEMO_INNER_RESOURCE "")

if(MORTY_BUILD_TARGET STREQUAL "IOS")
    set(DEMO_INNER_RESOURCE ${DEMO_INNER_RESOURCE}
        "iOS Launch Screen.storyboard"
    )
endif()

file(GLOB_RECURSE DEMO_All_FILES *.h *.cpp)
add_executable(Demo ${DEMO_All_FILES})

target_link_libraries(Demo Core)
target_link_libraries(Demo Render)
target_link_libraries(Demo Editor)

if(MORTY_BUILD_WITH_PHYSICS)
target_link_libraries(Demo Physics)
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

if(GENERATOR_FOR_CLANGD STREQUAL "ON")
    ADD_CUSTOM_COMMAND(TARGET Demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_BINARY_DIR}/../compile_commands.json"
        COMMENT "Copying 'compile_commands.json'")
endif()