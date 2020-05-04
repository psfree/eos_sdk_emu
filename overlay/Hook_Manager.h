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

#ifndef __INCLUDED_HOOK_BASE_H__
#define __INCLUDED_HOOK_BASE_H__

#include "Base_Hook.h"

#include <set>

class Hook_Manager
{
    friend class Base_Hook;

protected:
    std::set<Base_Hook*> _hooks; 

    Hook_Manager();
    virtual ~Hook_Manager();

public:
    static Hook_Manager& Inst();

    void AddHook(Base_Hook* hook);
    void RemoveHook(Base_Hook* hook);
};

#endif//__INCLUDED_HOOK_BASE_H__
