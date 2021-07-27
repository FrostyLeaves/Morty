add_library(Vulkan::Vulkan UNKNOWN IMPORTED)

set(Vulkan_INCLUDE_DIR ${THIRD_PARTY_PATH}/MoltenVK/MoltenVK/include)
set(Vulkan_LIBRARY ${THIRD_PARTY_PATH}/installs/MoltenVK/lib/Release/dynamic/libMoltenVK.dylib)

set_target_properties(Vulkan::Vulkan
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}"
                                IMPORTED_LOCATION "${Vulkan_LIBRARY}"
)

set(Vulkan_FOUND True)