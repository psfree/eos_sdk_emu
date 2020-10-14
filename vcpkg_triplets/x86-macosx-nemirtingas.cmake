set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_BUILD_TYPE release)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)

set(ENV{OSXCROSS_HOST} "i386-apple-$ENV{OSXCROSS_TARGET}")

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE /osxcross/target/toolchain.cmake)

set(ENV{VCPKG_TOOLCHAIN} "/vcpkg/scripts/toolchains/osx.cmake")

set(VCPKG_C_FLAGS "-stdlib=libc++")
set(VCPKG_CXX_FLAGS "-stdlib=libc++")