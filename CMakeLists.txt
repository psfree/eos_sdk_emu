#CMAKE_TOOLCHAIN_FILE

project(nemirtingas_epic_emu)
cmake_minimum_required(VERSION 3.0)

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
  elseif(X86)
    set(EPIC_API EOSSDK-Win32-Shipping)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()

  file(
    GLOB
    overlay_pf_sources
	overlay/Base_Hook.cpp
    #overlay/windows/*
    #extra/ImGui/impls/*.cpp
    #extra/ImGui/impls/windows/*
  )

elseif(APPLE)
  message(STATUS "CMake for APPLE is experimental")
  if(X64)
    # Global flags for building steamapi (64bits)
    set(CMAKE_EXE_LINKER_FLAGS -m64)
    set(CMAKE_SHARED_LINKER_FLAGS -m64)
    set(CMAKE_STATIC_LINKER_FLAGS -m64)
  elseif(X86)
    # Global flags for building steamapi (32bits)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
    set(CMAKE_SHARED_LINKER_FLAGS -m32)
    set(CMAKE_STATIC_LINKER_FLAGS -m32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  #add_compile_options("-stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")

  set(EPIC_API libEOSSDK-Mac-Shipping)

  #file(
  #  GLOB
  #  overlay_pf_sources
  #  overlay/macosx/*
  #)

elseif(UNIX)
  if(X64)
    # Global flags for building steamapi (64bits)
    set(CMAKE_EXE_LINKER_FLAGS -m64)
    set(CMAKE_SHARED_LINKER_FLAGS -m64)
    set(CMAKE_STATIC_LINKER_FLAGS -m64)
  elseif(X86)
    # Global flags for building steamapi (32bits)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
    set(CMAKE_SHARED_LINKER_FLAGS -m32)
    set(CMAKE_STATIC_LINKER_FLAGS -m32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  set(EPIC_API libEOSSDK-Linux-Shipping)

  #file(
  #  GLOB
  #  overlay_pf_sources
  #  overlay/linux/*
  #  extra/ImGui/impls/linux/*
  #)

else()
  message(FATAL_ERROR "No CMake for other platforms")

endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(Protobuf_SRC_ROOT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/protobuf-src/" CACHE PATH "Protobuf root folder")

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Debug or Release")

#option(GLEW_USE_STATIC_LIBS "Use GLEW static libs" ON)
#set(Glew_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/glew-src/include/" CACHE PATH "GLEW include folder")

#set(Curl_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/curl-src/include/" CACHE PATH "cURL include folder")

#set(ImGui_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/ImGui/" CACHE PATH "ImGui include folder")

set(JSON_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/nlohmann/" CACHE PATH "Json include folder")

#set(GAMEPAD_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/gamepad/" CACHE PATH "Gamepad include folder")

set(EOSSDK_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/eos_sdk/" CACHE PATH "Epic Online Service SDK include folder")

if(X64)
  if(APPLE)
    set(Protobuf_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/macosx64" CACHE PATH "Protobuf build directory")
    #set(Glew_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/macosx64" CACHE PATH "Glew build directory")
    #set(Curl_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/macosx64" CACHE PATH "cURL build directory")
  elseif(UNIX)
    set(Protobuf_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/linux64" CACHE PATH "Protobuf build directory")
    #set(Glew_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/linux64" CACHE PATH "Glew build directory")
    #set(Curl_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/linux64" CACHE PATH "cURL build directory")
  elseif(WIN32)
    set(Protobuf_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/win64" CACHE PATH "Protobuf build directory")
	#set(Glew_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/win64" CACHE PATH "Glew build directory")
	#set(Curl_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/win64" CACHE PATH "cURL build directory")
  endif()
elseif(X86)
  if(APPLE)
    set(Protobuf_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/macosx32" CACHE PATH "Protobuf build directory")
	#set(Glew_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/macosx32" CACHE PATH "Glew build directory")
	#set(Curl_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/macosx32" CACHE PATH "cURL build directory")
  elseif(UNIX)
    set(Protobuf_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/linux32" CACHE PATH "Protobuf build directory")
	#set(Glew_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/linux32" CACHE PATH "Glew build directory")
	#set(Curl_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/linux32" CACHE PATH "cURL build directory")
  elseif(WIN32)
    set(Protobuf_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/protobuf/win32" CACHE PATH "Protobuf build directory")
	#set(Glew_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/glew/win32" CACHE PATH "Glew build directory")
	#set(Curl_BUILD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/extra/curl/win32" CACHE PATH "cURL build directory")
  endif()
endif()

#set(CMAKE_INCLUDE_PATH "${Glew_INCLUDE_FOLDER};${Curl_INCLUDE_FOLDER};${ImGui_INCLUDE_FOLDER};${JSON_INCLUDE_FOLDER};${EOSSDK_INCLUDE_FOLDER};${CMAKE_INCLUDE_PATH};${GAMEPAD_INCLUDE_FOLDER}" CACHE PATH "Where to find prebuilt includes")
#set(CMAKE_PREFIX_PATH "${Protobuf_BUILD_FOLDER};${Glew_BUILD_FOLDER};${Curl_BUILD_FOLDER};${CMAKE_PREFIX_PATH}" CACHE PATH "Where to find prebuilt dependencies")

set(CMAKE_INCLUDE_PATH "${JSON_INCLUDE_FOLDER};${EOSSDK_INCLUDE_FOLDER};${CMAKE_INCLUDE_PATH}" CACHE PATH "Where to find prebuilt includes")
set(CMAKE_PREFIX_PATH "${Protobuf_BUILD_FOLDER};${CMAKE_PREFIX_PATH}" CACHE PATH "Where to find prebuilt dependencies")

include(FindProtobuf)
find_package(Protobuf REQUIRED)

#find_package(CURL REQUIRED)
#
#if(NOT APPLE)
#  find_package(GLEW REQUIRED)
#endif()

#find_path(
#  ImGui_INCLUDE
#  imgui.h
#)

find_path(
  JSON_INCLUDE
  json.hpp
)

#find_path(
#  GAMEPAD_INCLUDE
#  gamepad.h
#)

find_path(
  EOSSDK_INCLUDE
  eos_base.h
)

########################################
## global includes dir to add
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR} # For net.pb.h (built by protobuf_generate_cpp)
  ${Protobuf_INCLUDE_DIRS}    # For protobuf headers
)

########################################
## net.h net.cc
protobuf_generate_cpp(net_PROTO_SRCS net_PROTO_HDRS proto/network_proto.proto)

########################################
## steamclient[64].dll
file(
  GLOB
  emu_sources
  eos_dll/*.cpp
)

if(WIN32)
  file(
    GLOB
    detours_sources
    extra/detours/*.cpp
  )
endif()

#if(NOT APPLE)
#  file(
#    GLOB
#    overlay_sources
#    overlay/*.cpp
#    extra/ImGui/*.cpp
#    extra/ImGui/impls/*.cpp
#  )
#endif()

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

#file(
#  GLOB
#  gamepad_sources
#  extra/gamepad/gamepad.cpp
#)

########################################
## 

add_library(
  ${EPIC_API}
  SHARED
  ${emu_sources}
  ${net_PROTO_SRCS}
  
  ${socket_sources}
  ${utils_sources}
  ${gamepad_sources}
  
  ${overlay_sources}
  ${overlay_pf_sources}
  $<$<BOOL:${WIN32}>:${detours_sources}>
)

if(UNIX)
  SET_TARGET_PROPERTIES(${EPIC_API} PROPERTIES PREFIX "")
endif()

target_link_libraries(
  ${EPIC_API}
  ${Protobuf_LITE_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${CURL_LIBRARIES}

  # For gamepad open-source xinput
  #$<$<BOOL:${WIN32}>:setupapi>
  # For curl ssl		
  #$<$<BOOL:${WIN32}>:advapi32>
  #$<$<BOOL:${WIN32}>:crypt32>
  # For overlay opengl
  #$<$<BOOL:${WIN32}>:opengl32>
  # For library network
  $<$<BOOL:${WIN32}>:ws2_32>
  $<$<BOOL:${WIN32}>:iphlpapi>
  # For library play .wav
  #$<$<BOOL:${WIN32}>:winmm>
  $<$<BOOL:${WIN32}>:comdlg32>
  $<$<BOOL:${WIN32}>:shell32>
  $<$<BOOL:${WIN32}>:user32>

  # For library multithread
  $<$<BOOL:${UNIX}>:pthread>
  # For library .so/.dylib loading
  $<$<BOOL:${UNIX}>:dl>
  # For overlay opengl
  #$<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:GL>
  # For curl ssl
  #$<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:crypto>
  #$<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:ssl>
)

target_include_directories(
  ${EPIC_API}
  PRIVATE
  ${JSON_INCLUDE}
  ${EOSSDK_INCLUDE}
  ${GAMEPAD_INCLUDE}
  ${GLEW_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${ImGui_INCLUDE}
  
  extra/
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
  #GLEW_STATIC
  #GLEW_NO_GLU
  #CURL_STATICLIB

  $<$<BOOL:${UNIX}>:GNUC>

  $<$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>:_DEBUG>
  $<$<STREQUAL:${CMAKE_BUILD_TYPE},Release>:EMU_RELEASE_BUILD NDEBUG>
)

####################################"
## Targets link & compiler options
## For linux based builds, make install rules

#install(FILES Readme_release.txt DESTINATION release/)
#install(FILES files_example/steam_appid.EDIT_AND_RENAME.txt files_example/steam_interfaces.EXAMPLE.txt DESTINATION release/)
#install(DIRECTORY files_example/steam_settings.EXAMPLE DESTINATION release/)

##################
## Install rules
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

if(WIN32)
  if(X64)
    set(OUT_DIR win64)
  else()
    set(OUT_DIR win32)
  endif()
  
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
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
  if(APPLE)
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
