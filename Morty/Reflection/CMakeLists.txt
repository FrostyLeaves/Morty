cmake_minimum_required(VERSION 3.10)


############################################################
# add library
############################################################

set(REFLECTOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../Script/reflector/generate_reflector.py")

add_custom_target(Reflection
        COMMAND ${PYTHON_VENV_PATH}/Scripts/python ${REFLECTOR_SCRIPT} ${PROJECT_SOURCE_DIR}/Render ${CMAKE_BINARY_DIR}
)
