cmake_minimum_required (VERSION 3.10)

file(GLOB_RECURSE TEST_All_FILES *.h *.cpp)
add_executable(Test ${TEST_All_FILES})

target_link_libraries(Test Core)
target_link_libraries(Test Render)

if(MORTY_BUILD_WITH_PHYSICS)
target_link_libraries(Test Physics)
endif()
