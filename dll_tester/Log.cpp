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

#ifndef DISABLE_LOG

#include "Log.h"
#include <thread>
#include <mutex>
#include <fstream>
#include <cstdarg>
#include <Windows.h>

void default_log_func(Log::LogLevel lv, const char* log_message);

decltype(Log::_log_level) Log::_log_level = Log::LogLevel::OFF;
decltype(Log::_log_func)  Log::_log_func = default_log_func;

void default_log_func(Log::LogLevel lv, const char* log_message)
{
    static std::ofstream log_file;
    static std::once_flag log_file_once;

    std::call_once(log_file_once, []()
    {
        std::string exe_path;
        char pgm_path[4096];
        GetModuleFileNameA(nullptr, pgm_path, 4095);

        exe_path = pgm_path;
        exe_path = exe_path.substr(0, exe_path.rfind('\\') + 1);
        log_file.open(exe_path + "wrapper.log", std::ios::trunc | std::ios::out);
    });

    log_file << log_message;
    log_file.flush();
}

bool _trace(const char* format, va_list argptr)
{
    std::string fmt = format;
    if (*fmt.rbegin() != '\n')
        fmt += '\n';

    va_list argptr2;
    va_copy(argptr2, argptr);

    int len = vsnprintf(nullptr, 0, fmt.c_str(), argptr);
    char* buffer = new char[++len];

    vsnprintf(buffer, len, fmt.c_str(), argptr2);
    va_end(argptr);
    va_end(argptr2);

    Log::get_log_func()(Log::get_loglevel(), buffer);

    delete[]buffer;
    return true;
}

void Log::L(LogLevel lv, const char* format, ...)
{
    if (lv >= _log_level && _log_level < LogLevel::MAX)
    {
        va_list argptr;
        va_start(argptr, format);
        _trace(format, argptr);
    }
}

#endif
