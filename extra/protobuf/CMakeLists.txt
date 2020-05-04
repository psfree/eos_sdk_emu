#CMAKE_TOOLCHAIN_FILE

project(nemirtingas_emu_protobuf)
cmake_minimum_required(VERSION 3.0)

if(WIN32)
  # Detect arch on Windows
  if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(X64 ON)
  else()
    set(X86 ON)
  endif()

  if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
  else()
    add_compile_options(-std=c++11)
  endif()

  set(win_libs Iphlpapi ws2_32)
  if(X64)
    set(STEAM_NAME steam_api64)
  elseif(X86)
    set(STEAM_NAME steam_api)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()

elseif(APPLE)
  if(X64)
    set(CMAKE_C_FLAGS   "-m64")
    set(CMAKE_CXX_FLAGS "-m64")
  elseif(X86)
    set(CMAKE_C_FLAGS   "-m32")
    set(CMAKE_CXX_FLAGS "-m32")
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

elseif(UNIX)
  if(X64)
    set(CMAKE_C_FLAGS   "-m64")
    set(CMAKE_CXX_FLAGS "-m64")
  elseif(X86)
    set(CMAKE_C_FLAGS   "-m32")
    set(CMAKE_CXX_FLAGS "-m32")
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fvisibility=hidden -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC")
   
else()
  message(FATAL_ERROR "Platform not supported")
endif()

set(PROTOBUF_VERSION     "3.9.0")
set(PROTOBUF_DIR         ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTOBUF_RELEASE_URL "https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-cpp-${PROTOBUF_VERSION}.tar.gz")
set(PROTOBUF_URL_SRC     protobuf-${PROTOBUF_VERSION})
set(PROTOBUF_SRC         protobuf-src)

if( NOT EXISTS ${PROTOBUF_DIR}/protobuf-src )
  file(
    DOWNLOAD ${PROTOBUF_RELEASE_URL} ${PROTOBUF_DIR}/${PROTOBUF_URL_SRC}.tgz
    SHOW_PROGRESS
    EXPECTED_HASH MD5=5dcc4684b7e91c45fbb9e35622d85387
  )

  if( NOT EXISTS ${PROTOBUF_DIR}/${PROTOBUF_URL_SRC}.tgz )
    message(FATAL_ERROR "Download of protobuf failed")
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf ${PROTOBUF_URL_SRC}.tgz
    WORKING_DIRECTORY ${PROTOBUF_DIR}
  )

  file(REMOVE ${PROTOBUF_DIR}/${PROTOBUF_URL_SRC}.tgz)
  file(RENAME ${PROTOBUF_DIR}/${PROTOBUF_URL_SRC} "${PROTOBUF_SRC}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(${PROTOBUF_DIR}/protobuf-src/cmake)
