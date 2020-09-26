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
#include "frame_result.h"

class IRunCallback
{
public:
    // RunFrame is always called when the BGetCallback is called
    virtual bool CBRunFrame() = 0;
    // RunCallbacks is run when you sent a callback
    // True  = FrameResult_t has been filled with a result
    // False = FrameResult_t is not changed
    virtual bool RunCallbacks(pFrameResult_t res) = 0;
    // Free a callback after it has been called
    virtual void FreeCallback(pFrameResult_t res) = 0;
};

class Callback_Manager
{
    std::chrono::steady_clock::time_point _frame_start_time;
    std::chrono::milliseconds _max_tick_budget;

    std::set<IRunCallback*> _frames_to_run;
    std::map<IRunCallback*, std::list<pFrameResult_t>> _callbacks_to_run;
    //std::map<IRunFrame*, std::list<pFrameResult_t>> _next_callbacks_to_run;
    std::map<IRunCallback*, std::map<EOS_NotificationId, pFrameResult_t>> _notifications;

    std::recursive_mutex local_mutex;

public:
    
    Callback_Manager();
    ~Callback_Manager();

    void register_frame  (IRunCallback* obj);
    void unregister_frame(IRunCallback* obj);
    
    void register_callbacks  (IRunCallback* obj);
    void unregister_callbacks(IRunCallback* obj);
    
    bool add_callback(IRunCallback* obj, pFrameResult_t res);

    EOS_NotificationId add_notification(IRunCallback* obj, pFrameResult_t res);
    bool remove_notification(IRunCallback* obj, EOS_NotificationId id);
    void remove_all_notifications(IRunCallback* obj);
    pFrameResult_t get_notification(IRunCallback* obj, EOS_NotificationId id);
    std::vector<pFrameResult_t> get_notifications(IRunCallback* obj, int callback_id);
    
    
    void run_frames();
    void run_callbacks();

    inline void set_max_tick_budget(uint32_t milliseconds)
    {
        _max_tick_budget = std::chrono::milliseconds{ milliseconds };
    }

    inline void tick()
    {
        _frame_start_time = std::chrono::steady_clock::now();
        run_frames();
        run_callbacks();
    }
};