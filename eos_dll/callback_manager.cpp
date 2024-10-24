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

void Callback_Manager::register_frame(IRunCallback* obj)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    _frames_to_run.emplace(obj);
}

void Callback_Manager::unregister_frame(IRunCallback* obj)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    _frames_to_run.erase(obj);
}

void Callback_Manager::register_callbacks(IRunCallback* obj)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    _callbacks_to_run[obj];
}

void Callback_Manager::unregister_callbacks(IRunCallback* obj)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    auto it = _callbacks_to_run.find(obj);
    if (it != _callbacks_to_run.end())
        _callbacks_to_run.erase(it);
}

bool Callback_Manager::add_callback(IRunCallback* obj, pFrameResult_t res)
{
    //TRACE_FUNC();
    GLOBAL_LOCK();

    _callbacks_to_run[obj].push_back(res);
    return true;
}

EOS_NotificationId Callback_Manager::add_notification(IRunCallback* obj, pFrameResult_t res)
{
    //TRACE_FUNC();
    GLOBAL_LOCK();

    static EOS_NotificationId notif_id = 1;

    _notifications[obj][notif_id] = res;

    return notif_id++;
}

bool Callback_Manager::remove_notification(IRunCallback* obj, EOS_NotificationId id)
{
    //TRACE_FUNC();
    GLOBAL_LOCK();

    auto& notifs = _notifications[obj];
    auto it = notifs.find(id);
    if (it == notifs.end())
        return false;

    obj->FreeCallback(it->second);
    notifs.erase(it);

    return true;
}

void Callback_Manager::remove_all_notifications(IRunCallback* obj)
{
    auto it = _notifications.find(obj);
    if (it != _notifications.end())
    {
        for (auto& res : it->second)
        {
            obj->FreeCallback(res.second);
        }

        _notifications.erase(it);
    }
}

pFrameResult_t Callback_Manager::get_notification(IRunCallback* obj, EOS_NotificationId id)
{
    auto& notifs = _notifications[obj];
    auto it = notifs.find(id);
    if (it != notifs.end())
        return it->second;

    return pFrameResult_t();
}

std::vector<pFrameResult_t> Callback_Manager::get_notifications(IRunCallback* obj, int callback_id)
{
    std::vector<pFrameResult_t> results;

    auto& notifs = _notifications[obj];

    results.reserve(notifs.size()); // Reserve the maximum we would need, avoid reallocations
    for (auto& notifs : notifs)
    {
        if (notifs.second->ICallback() == callback_id)
            results.push_back(notifs.second);
    }

    return results;
}

void Callback_Manager::run_frames()
{
    //TRACE_FUNC();
    GLOBAL_LOCK();

    for (auto& frame : _frames_to_run)
    {
        frame->CBRunFrame();
        //if (_max_tick_budget.count() && (std::chrono::steady_clock::now() - _frame_start_time) > _max_tick_budget)
        //{
        //    APP_LOG(Log::LogLevel::WARN, "Exiting because of budget");
        //    return;
        //}
    }

    GetNetwork().CBRunFrame(0);
}

void Callback_Manager::run_callbacks()
{
    //TRACE_FUNC();
    GLOBAL_LOCK();

    // For each callback registerer object
    for (auto& callback : _callbacks_to_run)
    {
        IRunCallback* frame = callback.first;
        std::list<pFrameResult_t>& results = callback.second;
        for (auto result_it = results.begin(); result_it != results.end();)
        {
            pFrameResult_t& res = *result_it;
            if (res->CallbackOKTimeout())
            {
                if (res->done || frame->RunCallbacks(res))
                {
                    APP_LOG(Log::LogLevel::DEBUG, "Callback ready: %s", get_callback_name(res->ICallback()).c_str());
                    if (res->GetFunc() != nullptr)
                        res->GetFunc()(res->GetFuncParam());

                    frame->FreeCallback(res);
                    result_it = results.erase(result_it);
                }
                else
                    ++result_it;
            }
            else
                ++result_it;
        }
        //if (_max_tick_budget.count() && (std::chrono::steady_clock::now() - _frame_start_time) > _max_tick_budget)
        //{
        //    APP_LOG(Log::LogLevel::WARN, "Exiting because of budget");
        //    return;
        //}
    }
}