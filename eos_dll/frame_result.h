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

using CallbackFunc = void(EOS_CALL *)(void*);

struct CallbackMessage_t
{
    int callback_type_id;
    uint8_t* func_param;
    size_t func_param_size;
    CallbackFunc cb_func;
};

class FrameResult
{
    std::chrono::milliseconds ok_timeout;
    bool remove_on_timeout; // Remove the result if the api didn't read it fast enought
    CallbackMessage_t res;
public:
    const std::chrono::time_point<std::chrono::steady_clock> created_time;
    bool done;    // Set this to true will tell the callback_manager to fire the callback/apicall

    FrameResult();
    FrameResult(FrameResult const& other);
    ~FrameResult();

    void* AllocCallback(CallbackFunc func, size_t func_param_size, int i_callback, std::chrono::milliseconds ok_timeout = std::chrono::milliseconds(100));
    void SetCallback(CallbackFunc func, uint8_t* func_param, size_t func_param_size, int i_callback, std::chrono::milliseconds ok_timeout = std::chrono::milliseconds(100));

    inline CallbackMessage_t const& GetCallbackMsg() const { return res; }
    inline int ICallback() const { return res.callback_type_id; }
    inline void* GetFuncParam() const { return res.func_param; }
    inline size_t CallbackSize() const { return res.func_param_size; }
    inline CallbackFunc GetFunc() const { return res.cb_func; }

    inline bool CallbackOKTimeout() { return ((std::chrono::steady_clock::now() - created_time) >= ok_timeout); }

    template<typename T>
    inline void SetCallback(CallbackFunc func, T* func_param, std::chrono::milliseconds ok_timeout = std::chrono::milliseconds(100))
    {
        SetCallback(func, reinterpret_cast<uint8_t*>(func_param), sizeof(T), T::k_iCallback, ok_timeout);
    }

    template<typename T>
    inline T& GetCallback()
    {
        assert((res.callback_type_id == T::k_iCallback));
        return *reinterpret_cast<T*>(res.func_param);
    }

    template<typename T>
    inline T& CreateCallback(CallbackFunc func, std::chrono::milliseconds ok_timeout = std::chrono::milliseconds(100))
    {
        void* buff = AllocCallback(func, sizeof(T), T::k_iCallback, ok_timeout);
        T* cb = new (buff) T;
        return *cb;
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
    // Free a callback after it has been called
    virtual void FreeCallback(pFrameResult_t res) = 0;
};