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
#include "eossdk_platform.h"

namespace sdk
{

EOSSDK_UI::EOSSDK_UI()
{}

EOSSDK_UI::~EOSSDK_UI()
{}

/**
 * The UI Interface is used to access the overlay UI.  Each UI component will have a function for
 * opening it.  All UI Interface calls take a handle of type EOS_HUI as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetUIInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetUIInterface
 */

 /**
  * Opens the overlay with a request to show the friends list.
  *
  * @param Options Structure containing the account id of the friends list to show.
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
  * @param CompletionDelegate A callback that is fired when the request to show the friends list has been sent to the overlay, or on an error.
  *
  * @return EOS_Success If the overlay has been notified about the request.
  *         EOS_InvalidParameters If any of the options are incorrect.
  *         EOS_NotConfigured If the overlay is not properly configured.
  *         EOS_NoChange If the overlay is already visible.
  */
void EOSSDK_UI::ShowFriends(const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_UI_ShowFriendsCallbackInfo& sfci = res->CreateCallback<EOS_UI_ShowFriendsCallbackInfo>((CallbackFunc)CompletionDelegate);
    sfci.ClientData = ClientData;
    sfci.LocalUserId = Options->LocalUserId;
    sfci.ResultCode = EOS_EResult::EOS_Success;

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Hides the active overlay.
 *
 * @param Options Structure containing the account id of the browser to close.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate A callback that is fired when the request to hide the friends list has been processed, or on an error.
 *
 * @return EOS_Success If the overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the overlay is not properly configured.
 *         EOS_NoChange If the overlay is already hidden.
 */
void EOSSDK_UI::HideFriends(const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_UI_HideFriendsCallbackInfo& hfci = res->CreateCallback<EOS_UI_HideFriendsCallbackInfo>((CallbackFunc)CompletionDelegate);
    hfci.ClientData = ClientData;
    hfci.LocalUserId = Options->LocalUserId;
    hfci.ResultCode = EOS_EResult::EOS_Success;

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Gets the friends overlay visibility.
 *
 * @param Options Structure containing the account id of the overlay owner.
 *
 * @return EOS_TRUE If the overlay is visible.
 */
EOS_Bool EOSSDK_UI::GetFriendsVisible(const EOS_UI_GetFriendsVisibleOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return EOS_FALSE;

    return EOS_FALSE;
}

/**
 * Register to receive notifications when the overlay display settings are updated.
 * Newly registered handlers will always be called the next tick with the current state.
 * @note must call RemoveNotifyDisplaySettingsUpdated to remove the notification.
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the NotificationFn.
 * @param Notification A callback that is fired when the overlay display settings are updated.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_UI::AddNotifyDisplaySettingsUpdated(const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options, void* ClientData, const EOS_UI_OnDisplaySettingsUpdatedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);
    EOS_UI_OnDisplaySettingsUpdatedCallbackInfo& odsuci = res->CreateCallback<EOS_UI_OnDisplaySettingsUpdatedCallbackInfo>((CallbackFunc)NotificationFn);
    odsuci.ClientData = ClientData;
    odsuci.bIsExclusiveInput = false;
    odsuci.bIsVisible = false;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when the overlay display settings are updated.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_UI::RemoveNotifyDisplaySettingsUpdated(EOS_NotificationId Id)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, Id);
}

/**
 * Updates the current Toggle Friends Key.  This key can be used by the user to toggle the friends
 * overlay when available. The default value represents `shift + tab` as `((int32_t)EOS_UIK_EShift | (int32_t)EOS_UIK_ETab)`.
 * The provided key should be a single key with zero or more modifier keys.  It should satisfy
 * EOS_UI_IsValidKeyCombination.
 *
 * @param Options Structure containing the key combination to use.
 *
 * @return EOS_Success If the overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the overlay is not properly configured.
 *         EOS_NoChange If the key combination did not change.
 */
EOS_EResult EOSSDK_UI::SetToggleFriendsKey(const EOS_UI_SetToggleFriendsKeyOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr || !IsValidKeyCombination(Options->KeyCombination))
        return EOS_EResult::EOS_InvalidParameters;

    return EOS_EResult::EOS_Success;
}

/**
 * Returns the current Toggle Friends Key.  This key can be used by the user to toggle the friends
 * overlay when available. The default value represents `shift + tab` as `((int32_t)EOS_UIK_EShift | (int32_t)EOS_UIK_ETab)`.
 *
 * @param Options Structure containing any options that are needed to retrieve the key.
 * @return A valid key combination which represent a single key with zero or more modifier keys.
 *		EOS_UIK_ENone will be returned if any error occurs.
 */
EOS_UI_EKeyCombination EOSSDK_UI::GetToggleFriendsKey(const EOS_UI_GetToggleFriendsKeyOptions* Options)
{
    TRACE_FUNC();

    return EOS_UI_EKeyCombination::EOS_UIK_ModifierShift | EOS_UI_EKeyCombination::EOS_UIK_F2;
}

/**
 * Determine if a key combination is valid. Key combinations must have a key type and optional modifiers. Key combinations consisting of only modifiers are invalid.
 *
 * @param KeyCombination The key to test.
 * @return  EOS_TRUE if the provided key combination is valid.
 */
EOS_Bool EOSSDK_UI::IsValidKeyCombination(EOS_UI_EKeyCombination KeyCombination)
{
    TRACE_FUNC();

    return (KeyCombination & EOS_UI_EKeyCombination::EOS_UIK_ValidModifierMask) == KeyCombination ? EOS_FALSE : EOS_TRUE;
}

/**
 * Define any preferences for any display settings.
 *
 * @param Options Structure containing any options that are needed to set
 * @return EOS_Success If the overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the overlay is not properly configured.
 *         EOS_NoChange If the preferences did not change.
 */
EOS_EResult EOSSDK_UI::SetDisplayPreference(const EOS_UI_SetDisplayPreferenceOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Returns the current notification location display preference.
 * @return The current notification location display preference.
 */
EOS_UI_ENotificationLocation EOSSDK_UI::GetNotificationLocationPreference()
{
    TRACE_FUNC();

    return EOS_UI_ENotificationLocation::EOS_UNL_BottomLeft;
}

EOS_EResult EOSSDK_UI::AcknowledgeEventId(const EOS_UI_AcknowledgeEventIdOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_UI::CBRunFrame()
{
    return false;
}

bool EOSSDK_UI::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_UI::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    //switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case callback_type::k_iCallback:
        //{
        //    callback_type& callback = res->GetCallback<callback_type>();
        //    // Free resources
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

}