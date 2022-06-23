
############################################################
# flatbuffer
############################################################

add_library(flatbuffers::flatbuffers UNKNOWN IMPORTED)

set_target_properties(flatbuffers::flatbuffers
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${THIRD_PARTY_PATH}/flatbuffers/include"
                                IMPORTED_LOCATION "${THIRD_PARTY_PATH}/installs/flatbuffers/lib/Release-iphoneos/libflatbuffers.a"
)

set(flatbuffers_FOUND True)
