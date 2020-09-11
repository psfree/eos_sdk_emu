/*
 * Copyright (C) 2019-2020 Nemirtingas
 * This file is part of the Nemirtingas's Steam Emulator
 *
 * The Nemirtingas's Steam Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The Nemirtingas's Steam Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Steam Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <thread>

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

    using log_func_t = void(*)(void* user_param, Log::LogLevel lv, const char* log_message);

    static void default_log_func(void* user_param, Log::LogLevel lv, const char* log_message);

private:
    static void*           _log_user_param;
    static Log::LogLevel   _log_level;
    static Log::log_func_t _log_func;

    Log()                      = delete;
    Log(Log const&)            = delete;
    Log(Log&&)                 = delete;
    Log& operator=(Log const&) = delete;
    Log& operator=(Log&&)      = delete;

    static bool _trace(const char* format, va_list argptr);

    LogLevel _lv;
    std::string _func_name;

public:
#ifndef DISABLE_LOG
    Log(LogLevel lv, const char* func_name):
        _lv(lv),
        _func_name(func_name)
    {
        auto tid = std::this_thread::get_id();
        Log::Format(_lv, "(%lx)%s - %s ENTRY", *reinterpret_cast<uint32_t*>(&tid), Log::loglevel_to_str(_lv), _func_name.c_str());
    }

    ~Log()
    {
        auto tid = std::this_thread::get_id();
        Log::Format(_lv, "(%lx)%s - %s EXIT", *reinterpret_cast<uint32_t*>(&tid), Log::loglevel_to_str(_lv), _func_name.c_str());
    }

    static inline Log::LogLevel get_loglevel()
    {
        return _log_level;
}

    static inline void set_loglevel(LogLevel lv)
    {
        if (lv >= LogLevel::MIN && lv <= LogLevel::MAX)
            _log_level = lv;
    }

    static inline Log::log_func_t get_log_func()
    {
        return _log_func;
    }

    static inline log_func_t set_log_func(log_func_t log_func, void* user_param)
    {
        auto old_func = log_func;
        _log_func = log_func;
        _log_user_param = user_param;
        return old_func;
    }
#else
    STEAM_LOG(LogLevel lv, const char* func_name) {}
    ~Log() {}

    static void dummy_log_func(Log::LogLevel lv, const char* log_message) { (void)lv; (void)log_message; }
    static inline Log::LogLevel get_loglevel() { return LogLevel::OFF; }
    static inline void set_loglevel(LogLevel lv) {}
    static inline log_func_t get_log_func() { return dummy_log_func; }
    static inline log_func_t set_log_func(log_func_t log_func) { return dummy_log_func; }
#endif

    static inline const char* loglevel_to_str()
    {
        return loglevel_to_str(_log_level);
    }

#ifndef DISABLE_LOG
    static void Format(LogLevel lv, const char* format, ...);
#else
    static inline void Format(LogLevel lv, const char* format, ...) { }
#endif
};

#ifndef DISABLE_LOG
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

    #define APP_LOG(lv, fmt, ...) Log::Format(lv, "(%lx)%s - %s: " fmt, std::this_thread::get_id(), Log::loglevel_to_str(lv), __MY_FUNCTION__, ##__VA_ARGS__)
    #define TRACE_FUNC() Log __func_trace_log(Log::LogLevel::TRACE, __MY_FUNCTION__)
#else //!DISABLE_LOG
    #define APP_LOG(...)
    #define TRACE_FUNC()
#endif
