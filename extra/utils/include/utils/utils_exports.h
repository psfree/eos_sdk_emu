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

#include "utils_osdetector.h"

#ifdef __cplusplus
    #define UTILS_EXTERN_NONE
    #define UTILS_EXTERN_C   extern "C"
    #define UTILS_EXTERN_CXX extern
#else
    #define UTILS_EXTERN_NONE
    #define UTILS_EXTERN_C   extern
    #define UTILS_EXTERN_CXX #error "No C++ export in C"
#endif

#if defined(UTILS_OS_WINDOWS)

    #if defined(__clang__)

        #define UTILS_CALL_DEFAULT 
        #define UTILS_CALL_STDL    __stdcall
        #define UTILS_CALL_CDECL   __cdecl
        #define UTILS_CALL_FAST    __fastcall
        #define UTILS_CALL_THIS    __thiscall

        #define UTILS_MODE_DEFAULT
        #define UTILS_MODE_EXPORT  __declspec(dllexport)
        #define UTILS_MODE_IMPORT  __declspec(dllimport)
        #define UTILS_MODE_HIDDEN

        #define UTILS_HIDE_CLASS(keyword)                                         UTILS_EXTERN_NONE UTILS_MODE_HIDDEN keyword
        #define UTILS_HIDE_API(return_type, call_convention)                      UTILS_EXTERN_NONE UTILS_MODE_HIDDEN return_type call_convention
        #define UTILS_EXPORT_API(extern_type, return_type, mode, call_convention) extern_type       mode              return_type call_convention

    #else

        #define UTILS_CALL_DEFAULT 
        #define UTILS_CALL_STDL    __stdcall
        #define UTILS_CALL_CDECL   __cdecl
        #define UTILS_CALL_FAST    __fastcall
        #define UTILS_CALL_THIS    __thiscall

        #define UTILS_MODE_DEFAULT
        #define UTILS_MODE_EXPORT  __declspec(dllexport)
        #define UTILS_MODE_IMPORT  __declspec(dllimport)
        #define UTILS_MODE_HIDDEN 

        #define UTILS_HIDE_CLASS(keyword)                                         UTILS_EXTERN_NONE UTILS_MODE_HIDDEN keyword
        #define UTILS_HIDE_API(return_type, call_convention)                      UTILS_EXTERN_NONE UTILS_MODE_HIDDEN return_type call_convention
        #define UTILS_EXPORT_API(extern_type, return_type, mode, call_convention) extern_type       mode              return_type call_convention

    #endif

#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)

    #define UTILS_CALL_DEFAULT 
    #define UTILS_CALL_STD     __attribute__((stdcall))
    #define UTILS_CALL_CDECL   __attribute__((cdecl))
    #define UTILS_CALL_FAST    __attribute__((fastcall))
    #define UTILS_CALL_THIS    __attribute__((thiscall))

    #define UTILS_MODE_DEFAULT
    #define UTILS_MODE_EXPORT  __attribute__((visibility("default")))
    #define UTILS_MODE_IMPORT  __attribute__((visibility("default")))
    #define UTILS_MODE_HIDDEN  __attribute__((visibility("hidden")))

    #define UTILS_HIDE_CLASS(keyword)                                         UTILS_EXTERN_NONE keyword     UTILS_MODE_HIDDEN
    #define UTILS_HIDE_API(return_type, call_convention)                      UTILS_EXTERN_NONE return_type UTILS_MODE_HIDDEN call_convention
    #define UTILS_EXPORT_API(extern_type, return_type, mode, call_convention) extern_type       return_type mode              call_convention

    //#define LOCAL_API __attribute__((visibility ("internal")))

#endif

#ifdef UTILS_EXPORT_LIBRARY
void shared_library_load(void* hmodule);
void shared_library_unload(void* hmodule);
#endif
