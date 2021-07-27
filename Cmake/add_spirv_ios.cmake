############################################################
# SPIRV-Cross 
############################################################

set(SPIRV_CROSS_INCLUDE ${THIRD_PARTY_PATH}/SPIRV-Cross
                        ${THIRD_PARTY_PATH}/SPIRV-Cross/include/spirv_cross
)

set( MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
    ${SPIRV_CROSS_INCLUDE}
)

add_library(spirv-cross-core UNKNOWN IMPORTED)

set_target_properties(spirv-cross-core
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE}"
                                IMPORTED_LOCATION "${THIRD_PARTY_PATH}/installs/SPIRV-Cross/lib-arm64/libspirv-cross-core.a"
)

add_library(spirv-cross-glsl UNKNOWN IMPORTED)

set_target_properties(spirv-cross-glsl
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE}"
                                IMPORTED_LOCATION "${THIRD_PARTY_PATH}/installs/SPIRV-Cross/lib-arm64/libspirv-cross-glsl.a"
)

add_library(spirv-cross-cpp UNKNOWN IMPORTED)

set_target_properties(spirv-cross-cpp
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_CROSS_INCLUDE}"
                                IMPORTED_LOCATION "${THIRD_PARTY_PATH}/installs/SPIRV-Cross/lib-arm64/libspirv-cross-cpp.a"
)

############################################################
# glslang
############################################################
if(NOT TARGET GLSLang) # this if is required for subsequent runs of CMake

    set(GLSLANG_INCULDE
        "${THIRD_PARTY_PATH}/SPIRV-Cross/external/glslang/SPIRV"
        "${THIRD_PARTY_PATH}/SPIRV-Cross/external/glslang/glslang/Include"
        "${THIRD_PARTY_PATH}/SPIRV-Cross/external/glslang/glslang/Public"
        "${THIRD_PARTY_PATH}/SPIRV-Cross/external/glslang"
    )

    add_library(GLSLang::GLSLang UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GLSLang
                        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libglslang.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libglslang.a"
    )

    add_library(GLSLang::SPIRV UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPIRV
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libSPIRV.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libSPIRV.a"
    )

    add_library(GLSLang::GenericCodeGen UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GenericCodeGen
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libGenericCodeGen.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libGenericCodeGen.a"
    )

    add_library(GLSLang::HLSL UNKNOWN IMPORTED)

    set_target_properties(GLSLang::HLSL
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libHLSL.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libHLSL.a"
    )

    add_library(GLSLang::MachineIndependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::MachineIndependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libMachineIndependent.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libMachineIndependent.a"
    )
    
    add_library(GLSLang::OGLCompiler UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OGLCompiler
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libOGLCompiler.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libOGLCompiler.a"
    )
    
    add_library(GLSLang::OSDependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OSDependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libOSDependent.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libOSDependent.a"
    )
    
    add_library(GLSLang::SPVRemapper UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPVRemapper
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libSPVRemapper.a"
                                   IMPORTED_LOCATION_RELEASE "${THIRD_PARTY_PATH}/installs/glslang/lib-arm64/libSPVRemapper.a"
    )

    add_library(SPIRV::ALL INTERFACE IMPORTED)
    set_property(TARGET SPIRV::ALL PROPERTY INTERFACE_LINK_LIBRARIES
                    GLSLang::GLSLang
                    GLSLang::SPIRV
                    GLSLang::GenericCodeGen
                    GLSLang::HLSL
                    GLSLang::MachineIndependent
                    GLSLang::OGLCompiler
                    GLSLang::OSDependent
                    GLSLang::SPVRemapper
                    spirv-cross-core
                    spirv-cross-glsl
                    spirv-cross-cpp
    )

    target_link_libraries(Morty PRIVATE SPIRV::ALL)

set(SPIRV_CROSS_FOUND True)

endif()