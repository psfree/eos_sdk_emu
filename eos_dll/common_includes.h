/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Epic Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * The Nemirtingas's Epic Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Epic Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <utils.h>
#include <network_proto.pb.h>

#if defined(__WINDOWS__)
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <shlobj.h>   // (shell32.lib) Infos about current user folders
    #include <PathCch.h>  // (pathcch.lib)  Canonicalize path
    #include <WinSock2.h> // Include before iphlpapi to enable winsock2 functions
    #include <iphlpapi.h> // (iphlpapi.lib) Infos about ethernet interfaces

#elif defined(__LINUX__) || defined(__APPLE__)
    #if defined(__LINUX__)
        #include <sys/sysinfo.h> // Get uptime (second resolution)
    #else
        #include <sys/sysctl.h>
        #include <mach-o/dyld_images.h>
    #endif

    #include <sys/types.h>
    #include <sys/ioctl.h> // get iface broadcast
    #include <sys/stat.h>  // stats on a file (is directory, size, mtime)

    #include <dirent.h> // go open directories
    #include <dlfcn.h>  // dlopen (like dll for linux)
    #include <net/if.h>
    #include <ifaddrs.h>// getifaddrs

    #include <limits.h> // PATH_MAX
    #include <unistd.h>

#else
    #error "unknown arch"
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef EPIC_SHARED_LIBRARY
    #ifdef EPIC_EXPORT
        #define EXPORT_EPIC_API EXPORT_API(dllexport)
    #else
        #define EXPORT_EPIC_API EXPORT_API(dllimport)
    #endif
#else
    #define LOCAL_API
#endif

#define EOS_BUILD_DLL 1

// SDK includes
#include <eos_sdk.h>
#include <eos_logging.h>
#include <eos_version.h>

// SDK Struct implementations
#include "eos_epicaccountiddetails.h"

#include <nlohmann/json.hpp>
#include <nlohmann/fifo_map.hpp>

#include <utfcpp/utf8.h>

// Workaround to use fifo_map in json
// A workaround to use fifo_map as map, we are just ignoring the 'less' compare
template<class K, class V, class dummy_compare, class A>
using my_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using fifo_json = nlohmann::basic_json<my_fifo_map>;

#include <thread>
#include <mutex>
#include <limits>
#include <chrono>
#include <locale>
#include <codecvt>
#include <random>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <utility>
#include <type_traits>
#include <cstdarg>
#include <cassert>

#include <ipv4/tcp_socket.h>
#include <ipv4/udp_socket.h>

#include <mini_detour/mini_detour.h>

#include <file_manager.h>

#include "md5.h"
#include "os_funcs.h"
#include "Log.h"
#include "helper_funcs.h"
#include "frame_result.h"

static constexpr char emu_savepath[] = "NemirtingasEpicEmu";

#if defined(__WINDOWS_32__)
  #define _EMU_VARIANT_ "win32"
#elif defined(__WINDOWS_64__)
  #define _EMU_VARIANT_ "win64"
#elif defined(__LINUX_32__)
  #define _EMU_VARIANT_ "lin32"
#elif defined(__LINUX_64__)
  #define _EMU_VARIANT_ "lin64"
#elif defined(__APPLE_32__)
  #define _EMU_VARIANT_ "mac32"
#elif defined(__APPLE_64__)
  #define _EMU_VARIANT_ "mac64"
#else
  #define _EMU_VARIANT_ "unk"
#endif
constexpr char _EMU_VERSION_[] = "0.0.0" "-" _EMU_VARIANT_;
