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

#include "eossdk_ui.h"

using namespace sdk;

EOS_DECLARE_FUNC(void) EOS_UI_ShowFriends(EOS_HUI Handle, const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    pInst->ShowFriends(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    pInst->HideFriends(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options)
{
    if (Handle == nullptr)
        return EOS_FALSE;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->GetFriendsVisible(Options);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyDisplaySettingsUpdated(EOS_HUI Handle, const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options, void* ClientData, const EOS_UI_OnDisplaySettingsUpdatedCallback NotificationFn)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->AddNotifyDisplaySettingsUpdated(Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyDisplaySettingsUpdated(EOS_HUI Handle, EOS_NotificationId Id)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    pInst->RemoveNotifyDisplaySettingsUpdated(Id);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->SetToggleFriendsKey(Options);
}

EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options)
{
    if (Handle == nullptr)
        return EOS_UI_EKeyCombination::EOS_UIK_ModifierShift | EOS_UI_EKeyCombination::EOS_UIK_F2;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->GetToggleFriendsKey(Options);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination)
{
    if (Handle == nullptr)
        return EOS_FALSE;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->IsValidKeyCombination(KeyCombination);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->SetDisplayPreference(Options);
}

EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle)
{
    if (Handle == nullptr)
        return EOS_UI_ENotificationLocation::EOS_UNL_TopRight;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->GetNotificationLocationPreference();
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_AcknowledgeEventId(EOS_HUI Handle, const EOS_UI_AcknowledgeEventIdOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_UI*>(Handle);
    return pInst->AcknowledgeEventId(Options);
}