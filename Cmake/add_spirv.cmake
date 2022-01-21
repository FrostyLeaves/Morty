############################################################
# SPIRV-Cross 
# glslang
############################################################

if(NOT TARGET GLSLang) # this if is required for subsequent runs of CMake

    set(VULKAN_SDK $ENV{VULKAN_SDK})

    set(GLSLANG_INCULDE
        "${VULKAN_SDK}/Include/glslang/Include"
        "${VULKAN_SDK}/Include/glslang/Public"
        "${VULKAN_SDK}/Include/glslang/SPIRV"
    )

    set( SPIRV_INCLUDE
        "${VULKAN_SDK}/Include/spirv_cross"
    )

    set( DXC_INCLUDE
        "${VULKAN_SDK}/Include/dxc"
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


    add_library(SPIRV::Core UNKNOWN IMPORTED)

    set_target_properties(SPIRV::Core
                        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/spirv-cross-core${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}spirv-cross-core${PPOS_R}"
    )

    add_library(SPIRV::Glsl UNKNOWN IMPORTED)

    set_target_properties(SPIRV::Glsl
                PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}spirv-cross-glsl${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}spirv-cross-glsl${PPOS_R}"
    )

    add_library(SPIRV::Cpp UNKNOWN IMPORTED)

    set_target_properties(SPIRV::Cpp
                PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}spirv-cross-cpp${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}spirv-cross-cpp${PPOS_R}"
    )

    add_library(SPIRV::Tools UNKNOWN IMPORTED)

    set_target_properties(SPIRV::Tools
                PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}SPIRV-Tools${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}SPIRV-Tools${PPOS_R}"
    )

    add_library(SPIRV::ToolsOpt UNKNOWN IMPORTED)

    set_target_properties(SPIRV::ToolsOpt
                PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SPIRV_INCLUDE}"
                                IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}SPIRV-Tools-opt${PPOS_D}"
                                IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}SPIRV-Tools-opt${PPOS_R}"
    )

    
    add_library(GLSLang::GLSLang UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GLSLang
                        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/glslang${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}glslang${PPOS_R}"
    )

    add_library(GLSLang::SPIRV UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPIRV
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}SPIRV${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}SPIRV${PPOS_R}"
    )

    add_library(GLSLang::GenericCodeGen UNKNOWN IMPORTED)

    set_target_properties(GLSLang::GenericCodeGen
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}GenericCodeGen${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}GenericCodeGen${PPOS_R}"
    )

    add_library(GLSLang::HLSL UNKNOWN IMPORTED)

    set_target_properties(GLSLang::HLSL
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}HLSL${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}HLSL${PPOS_R}"
    )

    add_library(GLSLang::MachineIndependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::MachineIndependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}MachineIndependent${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}MachineIndependent${PPOS_R}"
    )
    
    add_library(GLSLang::OGLCompiler UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OGLCompiler
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}OGLCompiler${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}OGLCompiler${PPOS_R}"
    )
    
    add_library(GLSLang::OSDependent UNKNOWN IMPORTED)

    set_target_properties(GLSLang::OSDependent
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}OSDependent${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}OSDependent${PPOS_R}"
    )
    
    add_library(GLSLang::SPVRemapper UNKNOWN IMPORTED)

    set_target_properties(GLSLang::SPVRemapper
                   PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLSLANG_INCULDE}"
                                   IMPORTED_LOCATION_DEBUG "${VULKAN_SDK}/Lib/${PPRS}SPVRemapper${PPOS_D}"
                                   IMPORTED_LOCATION_RELEASE "${VULKAN_SDK}/Lib/${PPRS}SPVRemapper${PPOS_R}"
    )

    add_library(DXC::DXCompiler UNKNOWN IMPORTED)

    set_target_properties(DXC::DXCompiler
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${DXC_INCLUDE}"
                                    IMPORTED_LOCATION "${VULKAN_SDK}/Lib/dxcompiler.lib"
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
                    SPIRV::Core
                    SPIRV::Glsl
                    SPIRV::Cpp
                    SPIRV::Tools
                    SPIRV::ToolsOpt
                    DXC::DXCompiler
    )

set(SPIRV_CROSS_FOUND True)




endif()