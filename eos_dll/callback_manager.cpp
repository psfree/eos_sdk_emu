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

#include "callback_manager.h"
#include "network.h"
#include "eossdk_platform.h"

constexpr static std::chrono::seconds cleanup_timeout(60);

Callback_Manager::Callback_Manager()
{}

Callback_Manager::~Callback_Manager()
{
}

void Callback_Manager::register_frame(IRunFrame* obj)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    _frames_to_run.insert(obj);
}

void Callback_Manager::unregister_frame(IRunFrame* obj)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    _frames_to_run.erase(obj);
}

void Callback_Manager::register_callbacks(IRunFrame* obj)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    _callbacks_to_run[obj];
}

void Callback_Manager::unregister_callbacks(IRunFrame* obj)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    auto it = _callbacks_to_run.find(obj);
    if (it != _callbacks_to_run.end())
        _callbacks_to_run.erase(it);
}

bool Callback_Manager::add_callback(IRunFrame* obj, pFrameResult_t res)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    _callbacks_to_run[obj].push_back(res);
    return true;
}

EOS_NotificationId Callback_Manager::add_notification(IRunFrame* obj, pFrameResult_t res)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    static EOS_NotificationId notif_id = 1;

    _notifications[obj][notif_id] = res;

    return notif_id++;
}

bool Callback_Manager::remove_notification(IRunFrame* obj, EOS_NotificationId id)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    auto& notifs = _notifications[obj];
    auto it = notifs.find(id);
    if (it == notifs.end())
        return false;

    notifs.erase(it);

    return true;
}

std::vector<pFrameResult_t> Callback_Manager::get_notifications(IRunFrame* obj, int callback_id)
{
    std::vector<pFrameResult_t> results;

    auto& notifs = _notifications[obj];
    for (auto& notifs : notifs)
    {
        if (notifs.second->res.m_iCallback == callback_id)
            results.push_back(notifs.second);
    }

    return results;
}

void Callback_Manager::run_frames()
{
    //LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    for (auto& frame : _frames_to_run)
        frame->CBRunFrame();

    int network_runs = 0;// Eat only 300 network frames max so we don't freeze the callback loop
    while (GetNetwork().CBRunFrame(0) && network_runs++ < 300);
}

void Callback_Manager::run_callbacks()
{
    //LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    // For each callback registerer object
    for (auto& callback : _callbacks_to_run)
    {
        IRunFrame* frame = callback.first;
        std::list<pFrameResult_t>& results = callback.second;
        for (auto result_it = results.begin(); result_it != results.end();)
        {
            pFrameResult_t& res = *result_it;
            if (res->done || frame->RunCallbacks(res))
            {
                LOG(Log::LogLevel::DEBUG, "Callback ready: %s", get_callback_name(res->res.m_iCallback).c_str());
                res->res.cb_func(res->res.data);
                result_it = results.erase(result_it);
            }
            else
                ++result_it;
        }
    }
}