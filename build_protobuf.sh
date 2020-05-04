#! /bin/bash

function help
{
    echo "Usage: $0 build_type arch" >&2
    echo "  build_type: 'Release' or 'Debug'" >&2
    echo "  arch      : Any in 'linux32', 'linux64', 'macosx32', 'macosx64', 'win32', 'win64'" >&2
    echo "" >&2
    echo "If you want to cross compile, setup your CMake cross-compile env in an exported VAR called EXTRA_CMAKE_ENV" >&2
    echo 'For example: EXTRA_CMAKE_ENV=-DCMAKE_TOOLCHAIN_FILE=$(pwd)/osxcross/target/toolchain.cmake' >&2
    exit 1
}

# Build type (Debug or Release)
BUILD_TYPE="$1"
# where to build protobuf, must be win32 or win64
OUT_DIR="$2"

if [ "$(uname)" == "Darwin" ]; then
  CMAKE="/Applications/CMake.app/Contents/bin/cmake"
elif [ "$(uname)" == "Linux" ]; then
  CMAKE="$(which cmake)"
else
  echo "Don't know how to handle '$(uname)'" >&2
  exit 1
fi

[ "$OUT_DIR" != "win32" -a "$OUT_DIR" != "win64" -a "$OUT_DIR" != "linux32" -a "$OUT_DIR" != "linux64" -a "$OUT_DIR" != "macosx32" -a "$OUT_DIR" != "macosx64" ] && help
[ "$BUILD_TYPE" != "Debug" -a "$BUILD_TYPE" != "Release" ] && help

# Protobuf options
build_tests="-Dprotobuf_BUILD_TESTS=OFF"
# OFF for static build, ON for .dll
build_shared="-Dprotobuf_BUILD_SHARED_LIBS=OFF" 
# ZLIB Support
build_with_zlib="-Dprotobuf_WITH_ZLIB=OFF"

# My variable to decide if we build x86 or x64 in CMakeLists.txt
if [ "$OUT_DIR" == "win32" -o "$OUT_DIR" == "linux32" -o "$OUT_DIR" == "macosx32" ]; then
  custom_arch_var="-DX86=ON"
else
  custom_arch_var="-DX64=ON"
fi

build_type="-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
#BUILD_TOOL="Ninja"
#BUILD_CMD="ninja"
BUILD_TOOL="Unix Makefiles"
BUILD_CMD="make -j${JOBS-2}"

# EXTRA_CMAKE_ENV is set by setup_clang_win_env.sh to build for windows or macos
# You must run setup_clang_macos_env.sh before calling this script if you build for windows or macos

rm -rf "extra/protobuf/$OUT_DIR" &&
mkdir  "extra/protobuf/$OUT_DIR" &&
cd     "extra/protobuf/$OUT_DIR" &&

echo "${CMAKE} -G \"${BUILD_TOOL}\" $EXTRA_CMAKE_ENV \"$build_tests\" \"$build_shared\" \"$build_with_zlib\" \"$custom_arch_var\" \"$build_type\" .." &&
${CMAKE} -G "${BUILD_TOOL}" $EXTRA_CMAKE_ENV "$build_tests" "$build_shared" "$build_with_zlib" "$custom_arch_var" "$build_type" .. &&
$BUILD_CMD || exit 1

rm -rf protobuf-src CMakeFiles

exit 0
