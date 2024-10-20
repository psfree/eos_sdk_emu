#CMAKE_TOOLCHAIN_FILE

project(Socket)
cmake_minimum_required(VERSION 3.0)

if(NOT X64 AND NOT X86)
  option(BUILD_32BITS "Build 32bits (64bits is default)" OFF)
  if(${BUILD_32BIT})
    set(X64 OFF)
    set(X86 ON)
  else()
    set(X64 ON)
    set(X86 OFF)
  endif()
endif()

if(WIN32) # Setup some variables for Windows build
  # Detect arch on Windows
  if(NOT X64 AND NOT X86 )
    if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      set(X64 ON)
    else()
      set(X86 ON)
    endif()
  endif()

  if(MSVC) # If building with MSVC
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

elseif(APPLE)
  message(STATUS "CMake for APPLE is experimental")
  if(X64)
    # Global flags for building steamapi (64bits)
    set(CMAKE_EXE_LINKER_FLAGS -m64)
    set(CMAKE_SHARED_LINKER_FLAGS -m64)
  elseif(X86)
    # Global flags for building steamapi (32bits)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
    set(CMAKE_SHARED_LINKER_FLAGS -m32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  #add_compile_options("-stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

elseif(UNIX)
  if(X64)
    # Global flags for building steamapi (64bits)
    set(CMAKE_EXE_LINKER_FLAGS -m64)
    set(CMAKE_SHARED_LINKER_FLAGS -m64)
  elseif(X86)
    # Global flags for building steamapi (32bits)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
    set(CMAKE_SHARED_LINKER_FLAGS -m32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()

else()
  message(FATAL_ERROR "No CMake for other platforms")

endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
option(BUILD_STATIC "Build static or shared library" ON)
option(BUILD_TESTS "Build tests app" OFF)

file(
  GLOB
  Socket_sources
  src/common/*.cpp
)

if(${BLUETOOTH_SUPPORT})
  file(
    GLOB
    Socket_sources
    ${Socket_sources}
    src/bluetooth/*.cpp
  )
endif()

########################################
## socket(.a|.lib)

if(${BUILD_STATIC})
  add_library(
    Socket
    STATIC
    ${Socket_sources}
  )
else()
  add_library(
    Socket
    SHARED
    ${Socket_sources}
  )
endif()

target_link_libraries(
  Socket

  # Winsocks
  $<$<BOOL:${WIN32}>:ws2_32>
  # Windows Bluetooth
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bthprops>
 
  # Linux Bluetooth
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bluetooth>
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:dbus-1>
)

target_include_directories(
  Socket
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/include
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/include/dbus-1.0>
  $<$<AND:$<BOOL:X64>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/x86_64-linux-gnu/dbus-1.0/include>
  $<$<AND:$<BOOL:X86>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/i386-linux-gnu/dbus-1.0/include>
)

#set_target_properties(
#  Socket_static PROPERTIES PUBLIC_HEADER
#  ".h;.h"
#)

target_compile_options(
  Socket
  PRIVATE
  
  $<$<BOOL:${UNIX}>:-fPIC -fpermissive -fvisibility=hidden -Wl,--exclude-libs,ALL>
  $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X86}>>:-m32>
  $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X64}>>:-m64>
  
  $<$<AND:$<CONFIG:>,$<BOOL:${MSVC}>>:/MP>
  $<$<AND:$<CONFIG:Debug>,$<BOOL:${MSVC}>>:/MP>
  $<$<AND:$<CONFIG:Release>,$<BOOL:${MSVC}>>:/MP>
)

target_compile_definitions(
  Socket
  PRIVATE
  SOCKET_EXPORT
)

if(${BUILD_TESTS})
  add_executable(
    ipv4_tests
    tests/ipv4_tests.cpp
  )

  target_include_directories(
    ipv4_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/include
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/include/dbus-1.0>
    $<$<AND:$<BOOL:X64>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/x86_64-linux-gnu/dbus-1.0/include>
    $<$<AND:$<BOOL:X86>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/i386-linux-gnu/dbus-1.0/include>
  )

  target_link_libraries(
    ipv4_tests
    Socket
  
    # Winsocks
    $<$<BOOL:${WIN32}>:ws2_32>
    # Windows Bluetooth
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bthprops>
   
    # Linux Bluetooth
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bluetooth>
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:dbus-1>
  )

  target_compile_options(
    ipv4_tests
    PRIVATE
  
    $<$<BOOL:${UNIX}>:-fpermissive -fvisibility=hidden -Wl,--exclude-libs,ALL>
    $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X86}>>:-m32>
    $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X64}>>:-m64>
  
    $<$<AND:$<CONFIG:>,$<BOOL:${MSVC}>>:/MP>
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${MSVC}>>:/MP>
    $<$<AND:$<CONFIG:Release>,$<BOOL:${MSVC}>>:/MP>
  )
endif()

##################
## Install rules
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})


if(WIN32)
  if(X64)
    set(OUT_DIR win64)
  else()
    set(OUT_DIR win32)
  endif()
elseif(APPLE)
  if(X64)
    set(OUT_DIR macosx64)
  else()
    set(OUT_DIR macosx32)
  endif()
elseif(UNIX)
  if(X64)
    set(OUT_DIR linux64)
  else()
    set(OUT_DIR linux32)
  endif()
endif()

install(
  TARGETS Socket
  ARCHIVE DESTINATION release/${OUT_DIR}
  LIBRARY DESTINATION release/${OUT_DIR}
  RUNTIME DESTINATION release/${OUT_DIR}
#  PUBLIC_HEADER DESTINATION release/include/
)

INSTALL (
  DIRECTORY ${CMAKE_SOURCE_DIR}/include/
  DESTINATION release/include
  FILES_MATCHING PATTERN "*.h*"
                 PATTERN "*.inl*"
)
