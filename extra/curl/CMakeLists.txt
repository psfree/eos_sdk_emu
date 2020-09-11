#CMAKE_TOOLCHAIN_FILE

project(nemirtingas_emulator_curl)
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

  set(win_libs Iphlpapi ws2_32)
  if(NOT X64 AND NOT X86)
    message(FATAL_ERROR "Arch unknown")
  endif()
  
  set(CURL_STATIC_CRT  ON CACHE BOOL   "")
  set(CMAKE_USE_WINSSL ON CACHE BOOL   "")
  set(CURL_CA_PATH "none" CACHE STRING "")
  
elseif(APPLE)
  if(X64)
  elseif(X86)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  message(WARNING "Apple CMake is experimental")
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

elseif(UNIX)
  if(X64)
    set(CMAKE_C_FLAGS   "-m64 -fPIC")
    set(CMAKE_CXX_FLAGS "-m64 -fPIC")
  elseif(X86)
    set(CMAKE_C_FLAGS   "-m32 -fPIC")
    set(CMAKE_CXX_FLAGS "-m32 -fPIC")
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
else()
  message(FATAL_ERROR "Platform not supported")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CURL_VERSION     "7.65.3")
set(CURL_DIR         ${CMAKE_CURRENT_SOURCE_DIR})
set(CURL_RELEASE_URL "https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.xz")
set(CURL_SRC         curl-src)

if( NOT EXISTS ${CURL_DIR}/${CURL_SRC} )
  file(
    DOWNLOAD ${CURL_RELEASE_URL} ${CURL_DIR}/curl.tar.xz
    SHOW_PROGRESS
    EXPECTED_HASH MD5=7bd5b2ebfd3f591034eb8b55314d8c02
  )

  if( NOT EXISTS ${CURL_DIR}/curl.tar.xz )
    message(FATAL_ERROR "Download of curl failed")
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf curl.tar.xz
    WORKING_DIRECTORY ${CURL_DIR}
  )

  file(REMOVE ${CURL_DIR}/curl.tar.xz)
  file(RENAME ${CURL_DIR}/curl-${CURL_VERSION} "${CURL_SRC}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BUILD_TESTING     OFF CACHE BOOL "")
set(BUILD_CURL_EXE    OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(HTTP_ONLY         ON  CACHE BOOL "")
set(CURL_ZLIB         OFF CACHE BOOL "")

add_subdirectory(${CURL_DIR}/${CURL_SRC})
