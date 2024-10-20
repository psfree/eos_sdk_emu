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

#include <utils/utils_exports.h>

#ifdef UTILS_EXPORT_LIBRARY
#if defined(UTILS_OS_WINDOWS)

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            shared_library_load(hinstDLL);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            shared_library_unload(hinstDLL);
            break;
    }
    return TRUE;
}

#elif defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
#include <dlfcn.h>

__attribute__((constructor)) void __utils_library_load__()
{
    Dl_info infos;
    dladdr((void*)&__utils_library_load__, &infos);
    shared_library_load(infos.dli_fbase);
}

__attribute__((destructor)) void __utils_library_unload__()
{
    Dl_info infos;
    dladdr((void*)&__utils_library_load__, &infos);
    shared_library_unload(infos.dli_fbase);
}

#endif//defined(UTILS_OS_LINUX) || defined(UTILS_OS_APPLE)
#endif//defined(UTILS_EXPORT_LIBRARY)
