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

#include "eos_presence.h"

using namespace sdk;

/**
 * The Presence methods allow you to query, read other player's presence information, as well as to modify your own.
 *
 * QueryPresence must be called once per login, per remote user, before data will be available. It is currently only possible to query presence for
 * users that are on your friends list, all other queries will return no results.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 */

 /**
  * Query a user's presence. This must complete successfully before CopyPresence will have valid results. If HasPresence returns true for a remote
  * user, this does not need to be called.
  *
  * @param Options Object containing properties related to who is querying presence and for what user
  * @param ClientData Optional pointer to help track this request, that is returned in the completion callback
  * @param CompletionDelegate Pointer to a function that handles receiving the completion information
  */
EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
}

/**
 * Check if we already have presence for a user
 *
 * @param Options Object containing properties related to who is requesting presence and for what user
 * @return EOS_TRUE if we have presence for the requested user, or EOS_FALSE if the request was invalid or we do not have cached data
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const EOS_Presence_HasPresenceOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_TRUE;
}

/**
 * Get a user's cached presence object. If successful, this data must be released by calling EOS_Presence_Info_Release
 *
 * @param Options Object containing properties related to who is requesting presence and for what user
 * @param OutPresence A pointer to a pointer of Presence Info. If the returned result is success, this will be set to data that must be later released, otherwise this will be set to NULL
 * @return Success if we have cached data, or an error result if the request was invalid or we do not have cached data.
 *
 * @see EOS_Presence_Info_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Creates a presence modification handle. This handle can used to add multiple changes to your presence that can be applied with EOS_Presence_SetPresence.
 * The resulting handle must be released by calling EOS_PresenceModification_Release once it has been passed to EOS_Presence_SetPresence.
 *
 * @param Options Object containing properties related to the user modifying their presence
 * @param OutPresenceModificationHandle Pointer to a Presence Modification Handle to be set if successful
 * @return Success if we successfully created the Presence Modification Handle pointed at in OutPresenceModificationHandle, or an error result if the input data was invalid
 *
 * @see EOS_PresenceModification_Release
 * @see EOS_Presence_SetPresence
 * @see EOS_PresenceModification_SetStatus
 * @see EOS_PresenceModification_SetRawRichText
 * @see EOS_PresenceModification_SetData
 * @see EOS_PresenceModification_DeleteData
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Sets your new presence with the data applied to a PresenceModificationHandle. The PresenceModificationHandle can be released safely after calling this function.
 *
 * @param Options Object containing a PresenceModificationHandle and associated user data
 * @param ClientData Optional pointer to help track this request, that is returned in the completion callback
 * @param CompletionDelegate Pointer to a function that handles receiving the completion information
 *
 * @see EOS_Presence_CreatePresenceModification
 * @see EOS_PresenceModification_Release
 */
EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
}

/**
 * Register to receive notifications when presence changes. If the returned NotificationId is valid, you must call RemoveNotifyOnPresenceChanged when you no longer wish to
 * have your NotificationHandler called
 *
 * @param ClientData Data the is returned to when NotificationHandler is invoked
 * @param NotificationHandler The callback to be fired when a presence change occurs
 * @return Notification ID representing the registered callback if successful, an invalid NotificationId if not
 *
 * @see EOS_ENotificationId::EOS_NotificationId_Invalid
 * @see EOS_Presence_RemoveNotifyOnPresenceChanged
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return 0;
}

/**
 * Unregister a previously bound notification handler from receiving presence update notifications
 *
 * @param NotificationId The Notification ID representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
}

/**
 * Register to receive notifications when a user accepts a join game option via the social overlay.
 * @note must call RemoveNotifyJoinGameAccepted to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyJoinGameAccepted(EOS_HPresence Handle, const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return 0;
}

/**
 * Unregister from receiving notifications when a user accepts a join game option via the social overlay.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyJoinGameAccepted(EOS_HPresence Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
}

/**
 * Gets a join info custom game-data string for a specific user. This is a helper function for reading the presence data related to how a user can be joined.
 * Its meaning is entirely application dependent.
 *
 * This value will be valid only after a QueryPresence call has successfully completed.
 *
 * @param Options Object containing an associated user
 * @param OutBuffer The buffer into which the character data should be written.  The buffer must be long enough to hold a string of EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH.
 * @param InOutBufferLength Used as an input to define the OutBuffer length.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the location string was copied into the OutBuffer.
 *         EOS_Success if the information is available and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if there is user or the location string was not found.
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the location string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 *
 * @see EOS_PRESENCE_LOCATIONSTRING_MAX_LEN
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_GetJoinInfo(EOS_HPresence Handle, const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * To modify your own presence, you must call EOS_Presence_CreatePresenceModification to create a Presence Modification handle. To modify that handle, call
 * EOS_PresenceModification_* methods. Once you are finished, call EOS_Presence_SetPresence with your handle. You must then release your Presence Modification
 * handle by calling EOS_PresenceModification_Release.
 */

 /**
  * Modifies a user's online status to be the new state.
  *
  * @param Options Object containing properties related to setting a user's Status
  * @return Success if modification was added successfully, otherwise an error code related to the problem
  */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetStatusOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Modifies a user's Rich Presence string to a new state. This is the exact value other users will see
 * when they query the local user's presence.
 *
 * @param Options Object containing properties related to setting a user's RichText string
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_RICH_TEXT_MAX_VALUE_LENGTH
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetRawRichText(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetRawRichTextOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Modifies one or more rows of user-defined presence data for a local user. At least one InfoData object
 * must be specified.
 *
 * @param Options Object containing an array of new presence data.
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_DATA_MAX_KEYS
 * @see EOS_PRESENCE_DATA_MAX_KEY_LENGTH
 * @see EOS_PRESENCE_DATA_MAX_VALUE_LENGTH
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetDataOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Removes one or more rows of user-defined presence data for a local user. At least one DeleteDataInfo object
 * must be specified.
 *
 * @param Options Object containing an array of new presence data.
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_DATA_MAX_KEYS
 * @see EOS_PRESENCE_DATA_MAX_KEY_LENGTH
 * @see EOS_PRESENCE_DATA_MAX_VALUE_LENGTH
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_DeleteData(EOS_HPresenceModification Handle, const EOS_PresenceModification_DeleteDataOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Sets your new join info custom game-data string. This is a helper function for reading the presence data related to how a user can be joined.
 * Its meaning is entirely application dependent.
 *
 * @param Options Object containing a join info string and associated user data
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_JOIN_INFO_STRING_LENGTH
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetJoinInfo(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetJoinInfoOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Presence*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Release the memory associated with an EOS_Presence_Info structure and its sub-objects. This must be called on data retrieved from EOS_Presence_CopyPresence.
 * This can be safely called on a NULL presence info object.
 *
 * @param PresenceInfo The presence info structure to be release
 */
EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo)
{

}

/**
* Release the memory associated with an EOS_HPresenceModification handle. This must be called on Handles retrieved from EOS_Presence_CreatePresenceModification.
* This can be safely called on a NULL presence modification handle. This also may be safely called while a call to SetPresence is still pending.
*
* @param PresenceModificationHandle The presence modification handle to release
*
* @see EOS_Presence_CreatePresenceModification
*/
EOS_DECLARE_FUNC(void) EOS_PresenceModification_Release(EOS_HPresenceModification PresenceModificationHandle)
{

}