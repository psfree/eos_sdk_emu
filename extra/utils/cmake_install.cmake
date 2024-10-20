# Install script for directory: C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/utils")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/msys64/mingw64/bin/objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/utils" TYPE FILE FILES
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/class_enum.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/constexpr_count.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/endianness.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/istring.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/scoped_lock.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/strings.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/switchstr.hpp"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/utils_exports.h"
    "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/include/utils/utils_osdetector.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Utils/UtilsConfig.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Utils/UtilsConfig.cmake"
         "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/CMakeFiles/Export/lib/cmake/Utils/UtilsConfig.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Utils/UtilsConfig-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Utils/UtilsConfig.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Utils" TYPE FILE FILES "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/CMakeFiles/Export/lib/cmake/Utils/UtilsConfig.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/msys64/home/LBURNS42/nemirtingas_epic_emu-master/extra/utils/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
