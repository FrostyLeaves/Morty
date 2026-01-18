vcpkg_check_linkage(ONLY_DYNAMIC_LIBRARY)

set(key NOTFOUND)
if(VCPKG_TARGET_IS_WINDOWS)
	set(key "windows-${VCPKG_TARGET_ARCHITECTURE}")
elseif(VCPKG_TARGET_IS_OSX)
	set(key "macosx-${VCPKG_TARGET_ARCHITECTURE}")
elseif(VCPKG_TARGET_IS_LINUX)
	set(key "linux-${VCPKG_TARGET_ARCHITECTURE}")
endif()

set(ARCHIVE NOTFOUND)
# For convenient updates, use 
# vcpkg install shader-slang --cmake-args=-DVCPKG_SHADER_SLANG_UPDATE=1
if(key STREQUAL "windows-x64" OR VCPKG_SHADER_SLANG_UPDATE)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-windows-x86_64.zip"
		FILENAME "slang-${VERSION}-windows-x86_64.zip"
		SHA512 f51ecb2bdd6ab3fedf167d1c816719ae9e58aff9359945598efb3f8e46abb3cdbc35490df4047148a4367c71989cdf76dbe783ce806d60c373623f69e6f15b34
	)
endif()
if(key STREQUAL "windows-arm64" OR VCPKG_SHADER_SLANG_UPDATE)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-windows-aarch64.zip"
		FILENAME "slang-${VERSION}-windows-aarch64.zip"
		SHA512 f96d5fd919f07abaa5d4c183afb0e83c68197208c14e35b7a69c6b2f118996b086754d680de2bffd86f9fa7f6d06271196ded297cf365ef19b3a0ed8875099fb
	)
endif()
if(key STREQUAL "macosx-x64" OR VCPKG_SHADER_SLANG_UPDATE)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-macos-x86_64.zip"
		FILENAME "slang-${VERSION}-macos-x86_64.zip"
		SHA512 04c6f53a7015bb33329ac6c9802fa8bec5d7f92e01aa048d15b33ff68b97c958d5cd5d3000ca490f75b25653307610fcb17c511c645dbf5935900328d0c25695
	)
endif()
if(key STREQUAL "macosx-arm64" OR VCPKG_SHADER_SLANG_UPDATE)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-macos-aarch64.zip"
		FILENAME "slang-${VERSION}-macos-aarch64.zip"
		SHA512 ffa347f2fecde251b1a5abfb5c46e1a04d14105d010980818ea2166f9593c6462dc0a1622429445c06acc866ec816a2f0b74fe70dae6b417e2f0ee3ebb2274d6
	)
endif()
if(key STREQUAL "linux-x64" OR VCPKG_SHADER_SLANG_UPDATE)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-linux-x86_64.zip"
		FILENAME "slang-${VERSION}-linux-x86_64.zip"
		SHA512 753113ec4dfa5cd76aff6e1738cb8631e05af93915ac1c62220992ea04f9f457cac828883d59095cca94b0ffadbfe78a90107096effa053e9e21e0845e9ed6c5
	)
endif()
if(key STREQUAL "linux-arm64" OR VCPKG_SHADER_SLANG_UPDATE)
	vcpkg_download_distfile(
		ARCHIVE
		URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-linux-aarch64.zip"
		FILENAME "slang-${VERSION}-linux-aarch64.zip"
		SHA512 3d7d836b63749eb51c31b9a7904c027a57390f1d0123a598088e057b5d5fa918ffd0f56a1589578d7ccf5c47ddf6ce464395b6d434d070cac12f344f46586508
	)
endif()
if(NOT ARCHIVE)
	message(FATAL_ERROR "Unsupported platform. Please implement me!")
endif()

vcpkg_extract_source_archive(
	BINDIST_PATH
	ARCHIVE "${ARCHIVE}"
	NO_REMOVE_ONE_LEVEL
)

if(VCPKG_SHADER_SLANG_UPDATE)
	message(STATUS "All downloads are up-to-date.")
	message(FATAL_ERROR "Stopping due to VCPKG_SHADER_SLANG_UPDATE being enabled.")
endif()

file(GLOB slang_cmake_configs
		"${BINDIST_PATH}/cmake/*.cmake"
)

# install cmake files
set(SLANG_INSTALL_CMAKE_PATH "${CURRENT_PACKAGES_DIR}/share/slang")
file(INSTALL ${slang_cmake_configs} DESTINATION "${SLANG_INSTALL_CMAKE_PATH}")

# Redirect CURRENT_PACKAGES_DIR from '/share' to '/share/slang'
file(READ "${SLANG_INSTALL_CMAKE_PATH}/slangTargets.cmake" _contents)
STRING(REPLACE "get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)" "get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)\nget_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)" _contents "${_contents}")
file(WRITE "${SLANG_INSTALL_CMAKE_PATH}/slangTargets.cmake" "${_contents}")

# Redirect the installation directory of the executable files from 'bin' to 'tools'
file(READ "${SLANG_INSTALL_CMAKE_PATH}/slangTargets-release.cmake" _contents)
STRING(REPLACE "\${_IMPORT_PREFIX}/bin/slangc" "\${_IMPORT_PREFIX}/tools/slang/slangc" _contents "${_contents}")
STRING(REPLACE "\${_IMPORT_PREFIX}/bin/slangd" "\${_IMPORT_PREFIX}/tools/slang/slangd" _contents "${_contents}")
STRING(REPLACE "\${_IMPORT_PREFIX}/bin/slangi" "\${_IMPORT_PREFIX}/tools/slang/slangi" _contents "${_contents}")
file(WRITE "${SLANG_INSTALL_CMAKE_PATH}/slangTargets-release.cmake" "${_contents}")


file(GLOB libs
	"${BINDIST_PATH}/lib/*.lib"
	"${BINDIST_PATH}/lib/*.dylib"
	"${BINDIST_PATH}/lib/*.so"
)
file(INSTALL ${libs} DESTINATION "${CURRENT_PACKAGES_DIR}/lib")

file(GLOB dyn_libs
	"${BINDIST_PATH}/lib/*.dylib"
	"${BINDIST_PATH}/lib/*.so"
)

if(VCPKG_TARGET_IS_WINDOWS)
  file(GLOB dlls "${BINDIST_PATH}/bin/*.dll")
  list(APPEND dyn_libs ${dlls})
  file(INSTALL ${dlls} DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
endif()

if(NOT VCPKG_BUILD_TYPE)
  file(INSTALL "${CURRENT_PACKAGES_DIR}/lib" DESTINATION "${CURRENT_PACKAGES_DIR}/debug")
  if(VCPKG_TARGET_IS_WINDOWS)
    file(INSTALL "${CURRENT_PACKAGES_DIR}/bin" DESTINATION "${CURRENT_PACKAGES_DIR}/debug")
  endif()
endif()

# On macos, slang has signed their binaries
# vcpkg wants to be helpful and update the rpath as it moves binaries around but this 
# breaks the code signature and makes the binaries useless
# Removing the signature is rude so instead we will disable rpath fixup
if(VCPKG_TARGET_IS_OSX OR VCPKG_TARGET_IS_IOS)
  set(VCPKG_FIXUP_MACHO_RPATH OFF)
endif()

vcpkg_copy_tools(TOOL_NAMES slangc slangd slangi SEARCH_DIR "${BINDIST_PATH}/bin" DESTINATION "${CURRENT_PACKAGES_DIR}/tools/slang")

file(GLOB headers "${BINDIST_PATH}/include/*.h")
file(INSTALL ${headers} DESTINATION "${CURRENT_PACKAGES_DIR}/include")

vcpkg_install_copyright(
	FILE_LIST "${BINDIST_PATH}/LICENSE"
	COMMENT #[[ from README ]] [[
The Slang code itself is under the Apache 2.0 with LLVM Exception license.

Builds of the core Slang tools depend on the following projects, either automatically or optionally, which may have their own licenses:

* [`glslang`](https://github.com/KhronosGroup/glslang) (BSD)
* [`lz4`](https://github.com/lz4/lz4) (BSD)
* [`miniz`](https://github.com/richgel999/miniz) (MIT)
* [`spirv-headers`](https://github.com/KhronosGroup/SPIRV-Headers) (Modified MIT)
* [`spirv-tools`](https://github.com/KhronosGroup/SPIRV-Tools) (Apache 2.0)
* [`ankerl::unordered_dense::{map, set}`](https://github.com/martinus/unordered_dense) (MIT)

Slang releases may include [slang-llvm](https://github.com/shader-slang/slang-llvm) which includes [LLVM](https://github.com/llvm/llvm-project) under the license:

* [`llvm`](https://llvm.org/docs/DeveloperPolicy.html#new-llvm-project-license-framework) (Apache 2.0 License with LLVM exceptions)
]])
