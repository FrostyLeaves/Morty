vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/glslang
    REF 153064f
    SHA512 0f279f906e861edb43ee91b73879702d699598952c193ec8e6838834a6306cd678d4087e8bc70f0ce0bb450bf309fa9fb89b98e6487bfe59b6bc05fd9d83aa8d
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        opt ENABLE_OPT
        opt ALLOW_EXTERNAL_SPIRV_TOOLS
        tools ENABLE_GLSLANG_BINARIES
        rtti ENABLE_RTTI
)

if (ENABLE_GLSLANG_BINARIES)
    vcpkg_find_acquire_program(PYTHON3)
    get_filename_component(PYTHON_PATH ${PYTHON3} DIRECTORY)
    vcpkg_add_to_path("${PYTHON_PATH}")
endif ()

if (VCPKG_TARGET_IS_WINDOWS)
    set(PLATFORM_OPTIONS "-DOVERRIDE_MSVCCRT=OFF")
endif ()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_EXTERNAL=OFF
        -DENABLE_CTEST=OFF
        -DSKIP_GLSLANG_INSTALL=OFF
        ${FEATURE_OPTIONS}
        ${PLATFORM_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/glslang DO_NOT_DELETE_PARENT_CONFIG_PATH)
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake)
vcpkg_replace_string("${CURRENT_PACKAGES_DIR}/share/${PORT}/glslang-config.cmake"
    "${PACKAGE_PREFIX_DIR}/lib/cmake/glslang/glslang-targets.cmake"
    "${PACKAGE_PREFIX_DIR}/share/${PORT}/glslang-targets.cmake"
)

vcpkg_copy_pdbs()

if (ENABLE_GLSLANG_BINARIES)
    vcpkg_copy_tools(TOOL_NAMES glslang glslangValidator spirv-remap AUTO_CLEAN)
endif ()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
