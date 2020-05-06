/*
 * Copyright (C) 2019-2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * The Nemirtingas's Steam Emulator is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The Nemirtingas's Steam Emulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Nemirtingas's Steam Emulator; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <thread>
#include <future>
#include <chrono>

class task
{
    std::promise<void> _exit_token;
    std::future<void> _future;
    std::thread _thread;
    bool _used;

public:
    task():
        _future(_exit_token.get_future()),
        _used(false)
    {}

    task(task &&other):
        _exit_token(std::move(other._exit_token)),
        _future(std::move(other._future)),
        _thread(std::move(other._thread)),
        _used(std::move(other._used))
    {}

    task& operator=(task&& other)
    {
        _exit_token = std::move(other._exit_token);
        _future = std::move(other._future);
        _thread = std::move(other._thread);
        _used = std::move(other._used);
        return *this;
    }

    ~task()
    {
    }

    template<typename Function, typename... Args>
    bool run(Function&& f, Args&&... args)
    {
        if (!_used)
        {
            _used = true;
            _thread = std::thread(f, args...);
            return true;
        }
        return false;
    }

    bool want_stop()
    {
        if (_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return false;

        return true;
    }

    void stop()
    {
        _exit_token.set_value();
    }

    void join()
    {
        if(_thread.joinable())
            _thread.join();
    }
};