############################################################
# SPIRV-Cross 
# glslang
############################################################

if(NOT TARGET GLSLang) # this if is required for subsequent runs of CMake

    set(VULKAN_SDK $ENV{VULKAN_SDK})

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

    add_library(SPIRV::ALL INTERFACE IMPORTED)
    set_property(TARGET SPIRV::ALL PROPERTY INTERFACE_LINK_LIBRARIES
                    SPIRV::Core
                    SPIRV::Glsl
                    SPIRV::Cpp
                    SPIRV::Tools
                    SPIRV::ToolsOpt
    )

set(SPIRV_CROSS_FOUND True)

endif()