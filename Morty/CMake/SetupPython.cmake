#setup python venv
find_program(Python_EXECUTABLE NAMES python)

function(setup_python)
    if (NOT EXISTS (PYTHON_VENV_PATH))
        message(VERBOSE "${PYTHON_VENV_PATH} not found. run ${Python_EXECUTABLE} -m venv ${PYTHON_VENV_PATH}")
        execute_process(
                COMMAND ${Python_EXECUTABLE} -m venv ${PYTHON_VENV_PATH}
        )
        execute_process(
                COMMAND ${PYTHON_VENV_PATH}/Scripts/pip install git+https://github.com/sighingnow/libclang.git@4fb1d602b96d607e90c1050db290933631d83c6e
        )
    endif ()
endfunction()
