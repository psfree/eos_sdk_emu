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

#include "eossdk_leaderboards.h"

namespace sdk
{

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Leaderboards::CBRunFrame()
{
    return false;
}

bool EOSSDK_Leaderboards::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_Leaderboards::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Leaderboards::FreeCallback(pFrameResult_t res)
{

}

}