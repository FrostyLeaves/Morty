add_library(Vulkan::Vulkan UNKNOWN IMPORTED)

set(Vulkan_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK/include)
set(Vulkan_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/installs/MoltenVK/lib/Release/dynamic/libMoltenVK.dylib)

set_target_properties(Vulkan::Vulkan
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIR}"
                                IMPORTED_LOCATION "${Vulkan_LIBRARY}"
)

set(Vulkan_FOUND True)