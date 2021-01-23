add_library(Vulkan::Vulkan UNKNOWN IMPORTED)

set_target_properties(Vulkan::Vulkan
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/MoltenVK/include"
                                IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Release/MoltenVK/dylib/macOS/libMoltenVK.dylib"
                                IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Release/MoltenVK/dylib/macOS/libMoltenVK.dylib"
)

set(Vulkan_FOUND True)