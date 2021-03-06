﻿############################################################
# SPIRV-Cross 
############################################################
set(spirv_cross_core_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/share/spirv_cross_core/cmake)
set(spirv_cross_glsl_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/share/spirv_cross_glsl/cmake)
set(spirv_cross_cpp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/share/spirv_cross_cpp/cmake)

find_package(spirv_cross_core)
find_package(spirv_cross_glsl)
find_package(spirv_cross_cpp)

set( MORTY_CODE_DIRECTORIES ${MORTY_CODE_DIRECTORIES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/include/spirv_cross
)

target_link_directories(Morty
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/SPIRV-Cross/lib
)


############################################################
# glslang
############################################################
if(NOT TARGET GLSLang) # this if is required for subsequent runs of CMake

    set(GLSLANG_INCULDE
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include/glslang/SPIRV"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include/glslang/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include/glslang/Public"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/include"
    )

if(WIN32)
    set(PPRS "")
    set(PPOS_D "d.lib")
    set(PPOS_R ".lib")
elseif(APPLE)
    set(PPRS "lib")
    set(PPOS_D "d.a")
    set(PPOS_R ".a")
endif()

    
    add_library(GLSLang::GLSLang UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GLSLang
                        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}glslang${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}glslang${PPOS_R}"
    )

    add_library(GLSLang::SPIRV UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPIRV
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPIRV${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPIRV${PPOS_R}"
    )

    add_library(GLSLang::GenericCodeGen UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GenericCodeGen
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}GenericCodeGen${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}GenericCodeGen${PPOS_R}"
    )

    add_library(GLSLang::HLSL UNKNOWN IMPORTED)

    set_target_properties(GLSLang::HLSL
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}HLSL${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}HLSL${PPOS_R}"
    )

    add_library(GLSLang::MachineIndependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::MachineIndependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}MachineIndependent${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}MachineIndependent${PPOS_R}"
    )
    
    add_library(GLSLang::OGLCompiler UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OGLCompiler
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OGLCompiler${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OGLCompiler${PPOS_R}"
    )
    
    add_library(GLSLang::OSDependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OSDependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OSDependent${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}OSDependent${PPOS_R}"
    )
    
    add_library(GLSLang::SPVRemapper UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPVRemapper
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPVRemapper${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/glslang/lib/${PPRS}SPVRemapper${PPOS_R}"
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

set(SPIRV_CROSS_FOUND True)

endif()