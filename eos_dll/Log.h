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

#ifdef _DEBUG

#include <string>

class Log
{
public:
    enum class LogLevel : uint8_t
    {
        MIN = 0,
        TRACE = MIN,
        DEBUG,
        INFO,
        WARN,
        ERR,
        FATAL,
        OFF,
        MAX = OFF
    };

private:
    Log()                      = delete;
    Log(Log const&)            = delete;
    Log(Log&&)                 = delete;
    Log& operator=(Log const&) = delete;
    Log& operator=(Log&&)      = delete;
    ~Log()                     = delete;

public:
    static void set_loglevel(LogLevel lv);

    constexpr static const char* loglevel_to_str(LogLevel lv)
    {
        switch (lv)
        {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO : return "INFO";
            case LogLevel::WARN : return "WARN";
            case LogLevel::ERR  : return "ERR";
            case LogLevel::FATAL: return "FATAL";
            case LogLevel::OFF  : return "OFF";
            default             : return "UNKNOWN";
        }
    }

    static void L(LogLevel lv, const char* format, ...);
};

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
    #define __WINDOWS_64__
    #define __64BITS__
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define __WINDOWS_32__
    #define __32BITS__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
    #define __MY_FUNCTION__ __FUNCTION__
#else

inline std::string className(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.rfind("::");
    if (colons == std::string::npos)
        return "";
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.substr(begin, end);
}

#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)

inline std::string fq_func(std::string const& classname, std::string const& funcname)
{
    if (classname.empty())
        return funcname;

    return classname + "::" + funcname;
}

#define __MY_FUNCTION__ fq_func(__CLASS_NAME__, __FUNCTION__).c_str()

#endif

#define LOG(lv, fmt, ...) Log::L(lv, "(%lu)%s - %s: " fmt, std::this_thread::get_id(), Log::loglevel_to_str(lv), __MY_FUNCTION__, ##__VA_ARGS__)
#else //!_DEBUG
#define LOG(...)
#endif