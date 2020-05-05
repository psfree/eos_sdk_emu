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

using CallbackFunc = void(*)(void*);

struct CallbackMessage_t
{
    int m_iCallback;
    uint8_t* data;
    CallbackFunc cb_func;
};

struct FrameResult
{
    const std::chrono::time_point<std::chrono::steady_clock> created_time;
    std::chrono::milliseconds ok_timeout;
    bool done;    // Set this to true will tell the callback_manager to fire the callback/apicall
    bool network; // Used in the callback functions, not used by callback_manager
    bool remove_on_timeout; // Remove the result if the api didn't read it fast enought
    CallbackMessage_t res;

    FrameResult() :
        created_time(std::chrono::steady_clock::now()),
        ok_timeout(std::chrono::milliseconds(0)),
        done(false),
        network(false),
        remove_on_timeout(true),
        res({})
    {
    }

    ~FrameResult()
    {
        delete[] res.data;
    }

    inline bool CallbackOKTimeout()
    {
        return ((std::chrono::steady_clock::now() - created_time) >= ok_timeout);
    }


    template<typename T>
    inline void SetCallback(CallbackFunc func, T* cb)
    {
        delete[] res.data;
        res.m_iCallback = T::k_iCallback;
        res.cb_func = func;
        res.data = reinterpret_cast<uint8_t*>(cb);
    }

    template<typename T>
    inline T& GetCallback()
    {
        return *reinterpret_cast<T*>(res.data);
    }

    template<typename T>
    inline T& CreateCallback(CallbackFunc func, std::chrono::milliseconds ok_timeout = std::chrono::milliseconds(0))
    {
        this->ok_timeout = ok_timeout;
        T* cb = reinterpret_cast<T*>(new uint8_t[sizeof(T)]);
        SetCallback(func, cb);
        return *reinterpret_cast<T*>(res.data);
    }
};

using pFrameResult_t = std::shared_ptr<FrameResult>;

class IRunFrame
{
public:
    // RunFrame is always called when running callbacks
    virtual bool CBRunFrame() = 0;
    // RunNetwork is run if you register to a network message and we received that message
    virtual bool RunNetwork(Network_Message_pb const& msg) = 0;
    // RunCallbacks is run when you sent a callback
    // True  = FrameResult_t has been filled with a result
    // False = FrameResult_t is not changed
    virtual bool RunCallbacks(pFrameResult_t res) = 0;
};