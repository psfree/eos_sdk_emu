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

#ifndef __INCLUDED_EPICACCOUNTIDDETAILS_H__
#define __INCLUDED_EPICACCOUNTIDDETAILS_H__

#include "common_includes.h"

struct EOS_EpicAccountIdDetails
{
    std::recursive_mutex local_mutex;

    std::string _id;

    EOS_Bool IsValid();
    EOS_EResult ToString(char* outBuffer, int32_t* outBufferSize);
    void FromString(const char* accountIdStr);
};

#endif