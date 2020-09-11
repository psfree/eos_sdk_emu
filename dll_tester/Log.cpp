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
#include <fstream>
#include <cstdarg>

decltype(Log::_log_user_param) Log::_log_user_param;
decltype(Log::_log_level)      Log::_log_level = Log::LogLevel::OFF;
decltype(Log::_log_func)       Log::_log_func  = default_log_func;

void Log::default_log_func(void* user_param, Log::LogLevel lv, const char* log_message)
{
    static std::ofstream log_file("nemirtingassteamemu.log", std::ios::trunc | std::ios::out);

#if defined(__WINDOWS__)
    if (IsDebuggerPresent())
    {
        OutputDebugString(log_message);
    }
    else
    {
        static bool console = false;
        if (!console)
        {
            AllocConsole();
            freopen("CONOUT$", "w", stdout);
        }

        fprintf(stdout, "%s", log_message);
    }
#endif

    log_file << log_message;
    log_file.flush();
    fprintf(stderr, "%s", log_message);
}

bool Log::_trace(const char* format, va_list argptr)
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

    _log_func(_log_user_param, _log_level, buffer);

    delete[]buffer;
    return true;
}

void Log::Format(LogLevel lv, const char* format, ...)
{
    if (lv >= _log_level && _log_level < LogLevel::MAX)
    {
        va_list argptr;
        va_start(argptr, format);
        _trace(format, argptr);
    }
}

#endif
