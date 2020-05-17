#CMAKE_TOOLCHAIN_FILE

project(nemirtingas_emu_zstd)
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

    set(ZSTD_USE_STATIC_RUNTIME ON)
  else()
    add_compile_options(-std=c++11)
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

set(ZSTD_VERSION     "1.4.4")
set(ZSTD_DIR         ${CMAKE_CURRENT_SOURCE_DIR})
set(ZSTD_RELEASE_URL "https://github.com/facebook/zstd/archive/v${ZSTD_VERSION}/zstd-${ZSTD_VERSION}.tar.gz")
set(ZSTD_URL_SRC     zstd-${ZSTD_VERSION})
set(ZSTD_SRC         zstd-src)

if( NOT EXISTS ${ZSTD_DIR}/zstd-src )
  file(
    DOWNLOAD ${ZSTD_RELEASE_URL} ${ZSTD_DIR}/${ZSTD_URL_SRC}.tar.gz
    SHOW_PROGRESS
    EXPECTED_HASH MD5=532aa7b3a873e144bbbedd9c0ea87694
  )

  if( NOT EXISTS ${ZSTD_DIR}/${ZSTD_URL_SRC}.tar.gz )
    message(FATAL_ERROR "Download of protobuf failed")
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xf ${ZSTD_URL_SRC}.tar.gz
    WORKING_DIRECTORY ${ZSTD_DIR}
  )

  file(REMOVE ${ZSTD_DIR}/${ZSTD_URL_SRC}.tar.gz)
  file(RENAME ${ZSTD_DIR}/${ZSTD_URL_SRC} "${ZSTD_SRC}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(${ZSTD_DIR}/zstd-src/build/cmake)
