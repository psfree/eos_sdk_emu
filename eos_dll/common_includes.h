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
    #include <Windows.h>
    #include <shlobj.h>   // (shell32.lib) Infos about current user folders
    #include <PathCch.h>  // (pathcch.lib)  Canonicalize path
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

// SDK Struct implementations
#include "eos_epicaccountiddetails.h"

#include <nlohmann/json.hpp>
#include <nlohmann/fifo_map.hpp>

#include <utfcpp/utf8.h>

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

#include <ipv4/tcp_socket.h>
#include <ipv4/udp_socket.h>

#include "os_funcs.h"
#include "Log.h"
#include "helper_funcs.h"
#include "frame_result.h"
#include "callback_manager.h"

static constexpr char emu_savepath[] = "NemirtingasEpicEmu";