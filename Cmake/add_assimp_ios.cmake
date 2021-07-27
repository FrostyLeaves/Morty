
############################################################
# assimp
############################################################
add_library(assimp::assimp UNKNOWN IMPORTED)

set_target_properties(assimp::assimp
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${THIRD_PARTY_PATH}/assimp/include"
                                IMPORTED_LOCATION "${THIRD_PARTY_PATH}/assimp/lib/iOS/arm64/libassimp.a"
)

set(assimp_FOUND True)

