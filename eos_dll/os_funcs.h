/*
 * Copyright (C) 2019 Nemirtingas
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

#include "common_includes.h"

#if defined(__WINDOWS__)
    #define PATH_SEPARATOR '\\'
#elif defined(__LINUX__) || defined(__APPLE__)
    #define PATH_SEPARATOR '/'
#endif

/*
#include <iomanip>

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream,
  const std::chrono::time_point<Clock, Duration> &time_point) {
  const time_t time = Clock::to_time_t(time_point);
#if __GNUC__ > 4 || \
    ((__GNUC__ == 4) && __GNUC_MINOR__ > 8 && __GNUC_REVISION__ > 1)
  // Maybe the put_time will be implemented later?
  struct tm tm;
  localtime_r(&time, &tm);
  return stream << std::put_time(&tm, "%c"); // Print standard date&time
#else
  char buffer[26];
  ctime_r(&time, buffer);
  buffer[24] = '\0';  // Removes the newline that is added
  return stream << buffer;
#endif
}
*/

LOCAL_API std::chrono::system_clock::time_point get_boottime();
LOCAL_API std::chrono::microseconds get_uptime();

// Try to disable all online networking
LOCAL_API void disable_online_networking();
LOCAL_API void enable_online_networking();

// Get User env variable
LOCAL_API std::string get_env_var(std::string const& var);
// User appdata full path
LOCAL_API std::string get_userdata_path();
// Executable full path
LOCAL_API std::string get_executable_path();
// .dll, .so or .dylib full path
LOCAL_API std::string get_module_path();

struct iface_ip_t
{
    uint32_t ip;   // Host ordered
    uint32_t mask; // Host ordered
};
LOCAL_API std::vector<iface_ip_t> const& get_ifaces_ip();
LOCAL_API std::vector<PortableAPI::ipv4_addr> const& get_broadcasts();

// Filesystem related functions
LOCAL_API std::string clean_path(std::string const& path);
LOCAL_API std::string canonical_path(std::string const& path);
LOCAL_API bool create_folder(std::string const& folder);
LOCAL_API std::string get_path_folder(std::string const& path);
LOCAL_API bool delete_file(std::string const& path);
LOCAL_API std::vector<std::string> list_files(std::string const& path, bool recursive = false);
