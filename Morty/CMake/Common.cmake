
############################################################
# flatbuffers generator
############################################################
find_program(FLATBUFFERS_FLATC_EXECUTABLE NAMES flatc)
function(generate_flatbuffers_header LIBRARY_NAME)

    file(GLOB_RECURSE ALL_FBS_PATH *.fbs)

    get_target_property(ALL_LIBRARY ${LIBRARY_NAME} INTERFACE_LINK_LIBRARIES)
    set(ALL_LIBRARY ${ALL_LIBRARY} ${LIBRARY_NAME})

    set(FLATBUFFERS_INCLUDE_DIR)

    foreach (PRE_LIBRARY ${ALL_LIBRARY})
        if (NOT ${PRE_LIBRARY} MATCHES "LINK_ONLY")
            get_target_property(LIB_INCLUDE_PATH_LIST ${PRE_LIBRARY} INTERFACE_INCLUDE_DIRECTORIES)
            foreach (PRE_INCLUDE_PATH ${LIB_INCLUDE_PATH_LIST})
                set(FLATBUFFERS_INCLUDE_DIR ${FLATBUFFERS_INCLUDE_DIR} "-I" ${PRE_INCLUDE_PATH})
            endforeach ()
        endif ()
    endforeach ()

    message("[flatbuffers] ${FLATBUFFERS_FLATC_EXECUTABLE} ${FLATBUFFERS_INCLUDE_DIR} --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer/ ${SRC_FBS} --reflect-types --reflect-names --scoped-enums")
    foreach (SRC_FBS ${ALL_FBS_PATH})
        execute_process(
                COMMAND ${FLATBUFFERS_FLATC_EXECUTABLE} ${FLATBUFFERS_INCLUDE_DIR} --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/Flatbuffer/ ${SRC_FBS} --reflect-types --reflect-names --scoped-enums
        )
    endforeach ()
endfunction()

############################################################
# reflection generator
############################################################
function(generate_reflection LIBRARY_NAME)

    file(GLOB_RECURSE ALL_HEADER_PATH *.h)

    get_target_property(LIBRARY_SOURCE_DIR ${LIBRARY_NAME} SOURCE_DIR)

    set(REFLECTOR_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../Script/reflector/generate_reflector.py")

    #get_target_property(ALL_LIBRARY ${LIBRARY_NAME} INTERFACE_LINK_LIBRARIES)
    #set(ALL_LIBRARY ${ALL_LIBRARY} ${LIBRARY_NAME})
    #set(ALL_LIBRARY_INCLUDE_DIR)
    #foreach (PRE_LIBRARY ${ALL_LIBRARY})
    #    if (NOT ${PRE_LIBRARY} MATCHES "LINK_ONLY")
    #        get_target_property(LIB_INCLUDE_PATH_LIST ${PRE_LIBRARY} INTERFACE_INCLUDE_DIRECTORIES)
    #        foreach (PRE_INCLUDE_PATH ${LIB_INCLUDE_PATH_LIST})
    #            set(ALL_LIBRARY_INCLUDE_DIR ${ALL_LIBRARY_INCLUDE_DIR} "-I" ${PRE_INCLUDE_PATH})
    #        endforeach ()
    #    endif ()
    #endforeach ()

    message("[reflection] ${PYTHON_VENV_PATH}/Scripts/python ${REFLECTOR_SCRIPT} ${LIBRARY_SOURCE_DIR} ${CMAKE_BINARY_DIR} ")

    execute_process(
            COMMAND ${PYTHON_VENV_PATH}/Scripts/python ${REFLECTOR_SCRIPT} ${LIBRARY_SOURCE_DIR} ${CMAKE_BINARY_DIR}/../
    )

endfunction()
