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

#pragma once

#include "common_includes.h"

namespace sdk
{
    class EOSSDK_Lobby :
        public IRunFrame
    {
    public:
        // RunFrame is always called when running callbacks
        virtual bool CBRunFrame();
        // RunNetwork is run if you register to a network message and we received that message
        virtual bool RunNetwork(Network_Message_pb const& msg);
        // RunCallbacks is run when you sent a callback
        // True  = FrameResult_t has been filled with a result
        // False = FrameResult_t is not changed
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);
    };
}