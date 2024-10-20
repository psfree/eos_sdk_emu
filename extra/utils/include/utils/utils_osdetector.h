/* Copyright (C) Nemirtingas
 * This file is part of utils.
 *
 * utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with utils.  If not, see <https://www.gnu.org/licenses/>
 */

#pragma once

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
    #define UTILS_OS_WINDOWS
    #define UTILS_ARCH_X64
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define UTILS_OS_WINDOWS
    #define UTILS_ARCH_X86
#elif defined(__linux__) || defined(linux)
    #if defined(__x86_64__)
        #define UTILS_OS_LINUX
        #define UTILS_ARCH_X64
    #else
        #define UTILS_OS_LINUX
        #define UTILS_ARCH_X86
    #endif
#elif defined(__APPLE__)
    #if defined(__x86_64__)
        #define UTILS_OS_APPLE
        #define UTILS_ARCH_X64
    #else
        #define UTILS_OS_APPLE
        #define UTILS_ARCH_X86
    #endif
#else
    //#error "Unknown OS"
#endif