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

class Callback_Manager
{
    std::set<IRunFrame*> _frames_to_run;
    std::map<IRunFrame*, std::list<pFrameResult_t>> _callbacks_to_run;
    //std::map<IRunFrame*, std::list<pFrameResult_t>> _next_callbacks_to_run;
    std::map<IRunFrame*, std::map<EOS_NotificationId, pFrameResult_t>> _notifications;

    std::recursive_mutex local_mutex;

public:
    
    Callback_Manager();
    ~Callback_Manager();

    void register_frame  (IRunFrame* obj);
    void unregister_frame(IRunFrame* obj);
    
    void register_callbacks  (IRunFrame* obj);
    void unregister_callbacks(IRunFrame* obj);
    
    bool add_callback(IRunFrame* obj, pFrameResult_t res);

    EOS_NotificationId add_notification(IRunFrame* obj, pFrameResult_t res);
    bool remove_notification(IRunFrame* obj, EOS_NotificationId id);
    std::vector<pFrameResult_t> get_notifications(IRunFrame* obj, int callback_id);
    
    
    void run_frames();
    void run_callbacks();
};