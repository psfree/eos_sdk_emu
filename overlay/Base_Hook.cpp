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
 * License along with the Goldberg Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "Base_Hook.h"
#include "Hook_Manager.h"

#include <algorithm>

#if defined(__WINDOWS__)

#include <detours/detours.h>

Base_Hook::Base_Hook():
    _library(nullptr)
{}

Base_Hook::~Base_Hook()
{
    UnhookAll();
}

const char* Base_Hook::get_lib_name() const
{
    return "<no_name>";
}

void Base_Hook::BeginHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
}

void Base_Hook::EndHook()
{
    DetourTransactionCommit();
}

void Base_Hook::HookFunc(std::pair<void**, void*> hook)
{
    if( DetourAttach(hook.first, hook.second) == 0 )
        _hooked_funcs.emplace_back(hook);
}

void Base_Hook::UnhookAll()
{
    if (_hooked_funcs.size())
    {
        BeginHook();
        std::for_each(_hooked_funcs.begin(), _hooked_funcs.end(), [](std::pair<void**, void*>& hook) {
            DetourDetach(hook.first, hook.second);
            });
        EndHook();
        _hooked_funcs.clear();
    }
}

#elif defined(__LINUX__)

Base_Hook::Base_Hook():
    _library(nullptr)
{}

Base_Hook::~Base_Hook()
{
    UnhookAll();
}

const char* Base_Hook::get_lib_name() const
{
    return "<no_name>";
}

void Base_Hook::BeginHook()
{
}

void Base_Hook::EndHook()
{
}

void Base_Hook::HookFunc(std::pair<void**, void*> hook)
{
}

void Base_Hook::UnhookAll()
{
}

#endif
