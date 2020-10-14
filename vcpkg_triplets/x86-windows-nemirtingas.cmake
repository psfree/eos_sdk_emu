set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_BUILD_TYPE release)

set(ENV{HOST_ARCH} ${VCPKG_TARGET_ARCHITECTURE})

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE /clang_windows_sdk/clang-cl-msvc.cmake)

set(ENV{VCPKG_TOOLCHAIN} "/vcpkg/scripts/toolchains/windows.cmake")