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

#include "../eos_dll/common_includes.h"
#include "Hook_Manager.h"

Hook_Manager::Hook_Manager()
{}

Hook_Manager::~Hook_Manager()
{
    for (auto& i : _hooks)
        delete i;
}

Hook_Manager& Hook_Manager::Inst()
{
    static Hook_Manager hook;
    return hook;
}

void Hook_Manager::AddHook(Base_Hook* hook)
{
    _hooks.insert(hook);
}

void Hook_Manager::RemoveHook(Base_Hook* hook)
{
    auto it = _hooks.find(hook);
    if (it != _hooks.end())
    {
        delete hook;
        _hooks.erase(it);
    }
}
