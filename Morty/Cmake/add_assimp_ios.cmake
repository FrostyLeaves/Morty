
############################################################
# assimp
############################################################
add_library(assimp::assimp UNKNOWN IMPORTED)

set_target_properties(assimp::assimp
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/assimp/include"
                                IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/assimp/lib/iOS/arm64/libassimp.a"
)

set(assimp_FOUND True)

