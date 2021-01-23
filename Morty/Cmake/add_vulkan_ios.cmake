add_library(Vulkan::Vulkan UNKNOWN IMPORTED)

set_target_properties(Vulkan::Vulkan
                    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/External/Vulkan-Headers/include"
                                IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/MoltenVK/Package/Release/MoltenVK/MoltenVK.xcframework/ios-arm64/libMoltenVK.a"
)

set(Vulkan_FOUND True)