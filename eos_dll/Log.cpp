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

#ifdef LIBRARY_DEBUG

#include "Log.h"
#include "common_includes.h"

// D:\Jeux\Epic Games\SatisfactoryExperimental\FactoryGame\Binaries\Win64\FactoryGame-Win64-Shipping.exe
// FactoryGame -AUTH_LOGIN=unused -AUTH_PASSWORD=bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb -AUTH_TYPE=exchangecode -epicapp=CrabTest -epicenv=Prod -EpicPortal  -epicusername="a name" -epicuserid=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA -epiclocale=fr
// D:\Jeux\Epic Games\SatisfactoryExperimental\FactoryGame\Binaries\Win64

decltype(Log::_log_level) Log::_log_level = Log::LogLevel::OFF;

LOCAL_API bool _trace(const char* format, va_list argptr)
{
    //va_list argptr;
    //va_start(argptr, format);

    std::string fmt = format;
    if (*fmt.rbegin() != '\n')
        fmt += '\n';

#if defined(__WINDOWS__)
    if (IsDebuggerPresent())
    {
        va_list argptr2;
        va_copy(argptr2, argptr);
        int len = vsnprintf(nullptr, 0, fmt.c_str(), argptr);

        char* buffer = new char[++len];
        vsnprintf(buffer, len, fmt.c_str(), argptr2);

        va_end(argptr);
        va_end(argptr2);

        OutputDebugString(buffer);
        delete[]buffer;
    }
    else
    {
        static bool console = false;
        if (!console)
        {
            AllocConsole();
            freopen("CONOUT$", "w", stdout);
        }
        va_list argptr2;
        va_copy(argptr2, argptr);
        int len = vsnprintf(nullptr, 0, fmt.c_str(), argptr);

        char* buffer = new char[++len];
        vsnprintf(buffer, len, fmt.c_str(), argptr2);

        va_end(argptr);
        va_end(argptr2);

        std::cout << buffer;
    }
#endif
    {
        //vfprintf(stderr, fmt.c_str(), argptr);
    }

    return true;
}

const char* Log::loglevel_to_str()
{
    return loglevel_to_str(_log_level);
}

void Log::set_loglevel(LogLevel lv)
{
    if (lv >= LogLevel::MIN && lv <= LogLevel::MAX)
        _log_level = lv;
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