#setup python venv
find_program(Python_EXECUTABLE NAMES python)

function(setup_python)
    if (NOT EXISTS (PYTHON_VENV_PATH))
        message("${PYTHON_VENV_PATH} not found. run ${Python_EXECUTABLE} -m venv ${PYTHON_VENV_PATH}")
        execute_process(
                COMMAND ${Python_EXECUTABLE} -m venv ${PYTHON_VENV_PATH}
        )
        execute_process(
                COMMAND ${PYTHON_VENV_PATH}/Scripts/pip install libclang
        )
    endif ()
endfunction()
