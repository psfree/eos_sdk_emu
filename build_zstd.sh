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
build_shared="-DZSTD_BUILD_SHARED=OFF"
# OFF for static build, ON for .dll
build_programs="-DZSTD_BUILD_PROGRAMS=OFF" 

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

args=()
args+=("$build_shared")
args+=("$build_programs")
args+=("$custom_arch_var")
args+=("$build_type")

if [ "$BUILD_TYPE" == "Debug" ] && [ "$OUT_DIR" == "win32" -o "$OUT_DIR" == "win64" ]; then
  args[3]="-DCMAKE_BUILD_TYPE=Release"
  args+=("-DCICD_DEBUG=ON")
fi

rm -rf "extra/zstd/$OUT_DIR" &&
mkdir  "extra/zstd/$OUT_DIR" &&
cd     "extra/zstd/$OUT_DIR" &&

echo "${CMAKE} -G \"${BUILD_TOOL}\" $EXTRA_CMAKE_ENV \"$build_shared\" \"$build_programs\" \"$custom_arch_var\" \"$build_type\" .." &&
${CMAKE} -G "${BUILD_TOOL}" $EXTRA_CMAKE_ENV "${args[@]}" .. &&
# Workaround
sed -ri 's/(list\(REMOVE_DUPLICATES \$\{flag_var\}\))/#\1/' ../zstd-src/build/cmake/CMakeModules/AddZstdCompilationFlags.cmake &&
${CMAKE} -G "${BUILD_TOOL}" $EXTRA_CMAKE_ENV "${args[@]}" .. &&
####
$BUILD_CMD || exit 1

rm -rf zstd-src CMakeFiles

exit 0
