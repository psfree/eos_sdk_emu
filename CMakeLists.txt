#CMAKE_TOOLCHAIN_FILE

project(nemirtingas_epic_emu)
cmake_minimum_required(VERSION 3.0)

# Workaround because cross-compiling with CMake + vcpkg fucks up this variable
# making the 'find_*' functions/macros somehow fail to find the right arch.
# There are conditions like
#  CMAKE_SIZEOF_VOID_P = "4" then look for i386 libraries...
# or
#  CMAKE_SIZEOF_VOID_P = "8" then look for x86_64 libraries...
if(X86 AND NOT X64)
  set(CMAKE_SIZEOF_VOID_P "4")
elseif(X64 AND NOT X86)
  set(CMAKE_SIZEOF_VOID_P "8")
else()
  message(FATAL_ERROR "Please define either -DX86=ON or -DX64=ON")
endif()

if(WIN32) # Setup some variables for Windows build
  if(MSVC) # If building with MSVC
    add_definitions(-D_CRT_SECURE_NO_WARNINGS) # Disable warning about strncpy_s and his friends
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"msvcrtd.lib\"")       # Disable this linkage
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:\"msvcrtd.lib\"") # Disable this linkage
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) # Force to only build Debug & Release projects

    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()

  endif()

  if(X64)
    set(EPIC_API EOSSDK-Win64-Shipping)
	set(OUT_DIR win64)
  elseif(X86)
    set(EPIC_API EOSSDK-Win32-Shipping)
	set(OUT_DIR win32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()

elseif(APPLE)
  message(STATUS "CMake for APPLE is experimental")
  if(X64)
    # Global flags for building steamapi (64bits)
    set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -m64")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
	set(OUT_DIR macosx64)
  elseif(X86)
    # Global flags for building steamapi (32bits)
    set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
	set(OUT_DIR macosx32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  #add_compile_options("-stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")

  set(EPIC_API libEOSSDK-Mac-Shipping)

elseif(UNIX)
  if(X64)
    # Global flags for building steamapi (64bits)
    set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -m64")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
	set(OUT_DIR linux64)
  elseif(X86)
    # Global flags for building steamapi (32bits)
    set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
	set(OUT_DIR linux32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  set(EPIC_API libEOSSDK-Linux-Shipping)

else()
  message(FATAL_ERROR "No CMake for other platforms")

endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug or Release")

option(DISABLE_LOG "Disable all logging. Will reduce emu size and will speed it up (a bit)" OFF)

option(USE_ZSTD_COMPRESS "Use zstd to compress network messages" ON)

set(Protobuf_USE_STATIC_LIBS ON)
include(FindProtobuf)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)

if(USE_ZSTD_COMPRESS)
  find_package(zstd CONFIG REQUIRED)
endif()

########################################
## net.h net.cc
protobuf_generate_cpp(net_PROTO_SRCS net_PROTO_HDRS proto/network_proto.proto)

########################################
## steamclient[64].dll
file(
  GLOB
  emu_sources
  eos_dll/*.cpp
  managers/*.cpp
)

if(WIN32)
  file(
    GLOB
    detours_sources
    extra/detours/*.cpp
  )
endif()

file(
  GLOB
  socket_sources
  extra/Socket/src/common/*.cpp
  extra/Socket/src/ipv4/*.cpp
)

file(
  GLOB
  utils_sources
  extra/utils/src/*.cpp
)

file(
  GLOB
  mini_detour_sources
  extra/mini_detour/*.cpp
)

########################################
## 

add_library(
  ${EPIC_API}
  SHARED
  ${emu_sources}
  ${net_PROTO_SRCS}
  
  ${mini_detour_sources}
  ${socket_sources}
  ${utils_sources}
)

if(UNIX)
  SET_TARGET_PROPERTIES(${EPIC_API} PROPERTIES PREFIX "")
endif()

target_link_libraries(
  ${EPIC_API}
  protobuf::libprotobuf-lite
  Threads::Threads
  $<$<BOOL:${USE_ZSTD_COMPRESS}>:libzstd>

  # For library network
  $<$<BOOL:${WIN32}>:ws2_32>
  $<$<BOOL:${WIN32}>:iphlpapi>
  # For library play .wav
  #$<$<BOOL:${WIN32}>:winmm>
  $<$<BOOL:${WIN32}>:shell32>
  $<$<BOOL:${WIN32}>:user32>

  # For library .so/.dylib loading
  $<$<BOOL:${UNIX}>:dl>
)

target_include_directories(
  ${EPIC_API}
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  
  eos_dll/
  managers/
  
  extra/
  extra/eos_sdk/
  extra/mini_detour/
  extra/utils/include
  extra/Socket/include
)

target_compile_options(
  ${EPIC_API}
  PUBLIC
  
  $<$<BOOL:${UNIX}>:-fPIC -fpermissive -fvisibility=hidden -Wl,--exclude-libs,ALL>
  $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X86}>>:-m32>
  $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X64}>>:-m64>
  
  $<$<AND:$<CONFIG:>,$<BOOL:${MSVC}>>:/MP>
  $<$<AND:$<CONFIG:Debug>,$<BOOL:${MSVC}>>:/MP>
  $<$<AND:$<CONFIG:Release>,$<BOOL:${MSVC}>>:/MP>
)

target_compile_definitions(
  ${EPIC_API}
  PUBLIC
  __EXPORT_SYMBOLS__
  EPIC_SHARED_LIBRARY
  EPIC_EXPORT

  $<$<BOOL:${USE_ZSTD_COMPRESS}>:NETWORK_COMPRESS>
  
  $<$<BOOL:${UNIX}>:GNUC>

  $<$<BOOL:${DISABLE_LOG}>:DISABLE_LOG>
  $<$<STREQUAL:${CMAKE_BUILD_TYPE},Release>:EMU_RELEASE_BUILD NDEBUG>
)

##################
## Install rules
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

if(WIN32)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR CICD_DEBUG)
    install(
      TARGETS ${EPIC_API}
      RUNTIME DESTINATION debug/${OUT_DIR}
    )
  else()
    install(
      TARGETS ${EPIC_API}
      RUNTIME DESTINATION release/${OUT_DIR}
    )
  endif()

elseif(APPLE OR UNIX)
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    install(
      TARGETS ${EPIC_API}
      LIBRARY DESTINATION debug/${OUT_DIR}
    )
  else()
    install(
      TARGETS ${EPIC_API}
      LIBRARY DESTINATION release/${OUT_DIR}
    )
  endif()
  
endif()
