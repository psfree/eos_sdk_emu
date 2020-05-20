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
# where to build steam_api
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

if [ "$OUT_DIR" == "win32" -o "$OUT_DIR" == "linux32" -o "$OUT_DIR" == "macosx32" ]; then
  # My variable to decide if we build x86 or x64 in CMakeLists.txt
  custom_arch_var="-DX86=ON"
else
  # My variable to decide if we build x86 or x64 in CMakeLists.txt
  custom_arch_var="-DX64=ON"
fi

if [ "$(uname)" == "Linux" ]; then
  if [ "$OUT_DIR" == "win32" -o "$OUT_DIR" == "macosx32" ]; then
    protobuf_exec="-DProtobuf_PROTOC_EXECUTABLE=../extra/protobuf/linux32/bin/protoc"
  elif [ "$OUT_DIR" == "win64" -o "$OUT_DIR" == "macosx64" ]; then
    protobuf_exec="-DProtobuf_PROTOC_EXECUTABLE=../extra/protobuf/linux64/bin/protoc"
  fi
elif [ "$(uname)" == "Darwin" ]; then
  if [ "$OUT_DIR" == "win32" -o "$OUT_DIR" == "linux32" ]; then
    protobuf_exec="-DProtobuf_PROTOC_EXECUTABLE=../extra/protobuf/macosx32/bin/protoc"
  elif [ "$OUT_DIR" == "win64" -o "$OUT_DIR" == "linux64" ]; then
    protobuf_exec="-DProtobuf_PROTOC_EXECUTABLE=../extra/protobuf/macosx64/bin/protoc"
  fi
fi

build_type="-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

args=()
args+=("$custom_arch_var")
args+=("$protobuf_exec")
args+=("$build_type")
if [ "$BUILD_TYPE" == "Debug" ] && [ "$OUT_DIR" == "win32" -o "$OUT_DIR" == "win64" ]; then
  args[2]="-DCMAKE_BUILD_TYPE=Release"
  args+=("-DCICD_DEBUG=ON")
fi

# EXTRA_CMAKE_ENV is set by setup_clang_env.sh to build for windows.
# You must run setup_clang_env.sh before calling this script if you build for windows.

rm -rf "$OUT_DIR" && mkdir "$OUT_DIR" &&
cd "$OUT_DIR" &&
echo "${CMAKE} -G \"Unix Makefiles\" $EXTRA_CMAKE_ENV \"${args[@]}\" .." &&
${CMAKE} -G "Unix Makefiles" $EXTRA_CMAKE_ENV "${args[@]}" .. &&
make -j${JOBS-2} &&
make install || exit 1

exit 0
