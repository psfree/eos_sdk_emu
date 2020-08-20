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

#include "frame_result.h"

FrameResult::FrameResult():
    created_time(std::chrono::steady_clock::now()),
    ok_timeout(std::chrono::milliseconds(0)),
    done(false),
    remove_on_timeout(true),
    res({})
{
}

FrameResult::FrameResult(FrameResult const& other) :
    created_time(other.created_time),
    ok_timeout(other.ok_timeout),
    done(other.done),
    remove_on_timeout(other.remove_on_timeout),
    res({})
{
    res.cb_func = other.res.cb_func;
    res.callback_type_id = other.res.callback_type_id;
    res.func_param_size = other.res.func_param_size;
    res.func_param = new uint8_t[other.res.func_param_size];
    memcpy(res.func_param, other.res.func_param, other.res.func_param_size);
}

FrameResult::~FrameResult()
{
    delete[] res.func_param;
}

void* FrameResult::AllocCallback(CallbackFunc func, size_t func_param_size, int i_callback, std::chrono::milliseconds ok_timeout)
{
    uint8_t* buff = new uint8_t[func_param_size];
    SetCallback(func, buff, func_param_size, i_callback, ok_timeout);
    return buff;
}

void FrameResult::SetCallback(CallbackFunc func, uint8_t* func_param, size_t func_param_size, int i_callback, std::chrono::milliseconds ok_timeout)
{
    this->ok_timeout = ok_timeout;
    delete[]res.func_param;
    res.cb_func = func;
    res.callback_type_id = i_callback;
    res.func_param_size = func_param_size;
    res.func_param = func_param;
}