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

#include "eos_ui.h"

using namespace sdk;

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
EOS_DECLARE_FUNC(void) EOS_UI_ShowFriends(EOS_HUI Handle, const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
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
EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
}

/**
 * Gets the friends overlay visibility.
 *
 * @param Options Structure containing the account id of the overlay owner.
 *
 * @return EOS_TRUE If the overlay is visible.
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
    return EOS_FALSE;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
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
EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
    return EOS_UI_EKeyCombination::EOS_UIK_ModifierShift | EOS_UI_EKeyCombination::EOS_UIK_Tab;
}

/**
 * Determine if a key combination is valid. Key combinations must have a key type and optional modifiers. Key combinations consisting of only modifiers are invalid.
 *
 * @param KeyCombination The key to test.
 * @return  EOS_TRUE if the provided key combination is valid.
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
    return EOS_TRUE;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Returns the current notification location display preference.
 * @return The current notification location display preference.
 */
EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_UI*>(Handle);
    return EOS_UI_ENotificationLocation::EOS_UNL_BottomLeft;
}
