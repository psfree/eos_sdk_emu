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
#include "callback_manager.h"

namespace sdk
{
    class EOSSDK_UI :
        public IRunCallback
    {
    public:
        EOSSDK_UI();
        ~EOSSDK_UI();

        virtual bool CBRunFrame();
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void                         ShowFriends(const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate);
        void                         HideFriends(const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate);
        EOS_Bool                     GetFriendsVisible(const EOS_UI_GetFriendsVisibleOptions* Options);
        EOS_NotificationId           AddNotifyDisplaySettingsUpdated(const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options, void* ClientData, const EOS_UI_OnDisplaySettingsUpdatedCallback NotificationFn);
        void                         RemoveNotifyDisplaySettingsUpdated(EOS_NotificationId Id);
        EOS_EResult                  SetToggleFriendsKey(const EOS_UI_SetToggleFriendsKeyOptions* Options);
        EOS_UI_EKeyCombination       GetToggleFriendsKey(const EOS_UI_GetToggleFriendsKeyOptions* Options);
        EOS_Bool                     IsValidKeyCombination(EOS_UI_EKeyCombination KeyCombination);
        EOS_EResult                  SetDisplayPreference(const EOS_UI_SetDisplayPreferenceOptions* Options);
        EOS_UI_ENotificationLocation GetNotificationLocationPreference();
        EOS_EResult                  AcknowledgeEventId(const EOS_UI_AcknowledgeEventIdOptions* Options);
    };
}
