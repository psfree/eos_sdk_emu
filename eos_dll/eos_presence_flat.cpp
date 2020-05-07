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

#include "eossdk_presence.h"

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

EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    pInst->QueryPresence(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const EOS_Presence_HasPresenceOptions* Options)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    return pInst->HasPresence(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    return pInst->CopyPresence(Options, OutPresence);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    return pInst->CreatePresenceModification(Options, OutPresenceModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    pInst->SetPresence(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    return pInst->AddNotifyOnPresenceChanged(Options, ClientData, NotificationHandler);
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId NotificationId)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    pInst->RemoveNotifyOnPresenceChanged(NotificationId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyJoinGameAccepted(EOS_HPresence Handle, const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    return pInst->AddNotifyJoinGameAccepted(Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyJoinGameAccepted(EOS_HPresence Handle, EOS_NotificationId InId)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    pInst->RemoveNotifyJoinGameAccepted(InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_GetJoinInfo(EOS_HPresence Handle, const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    auto pInst = reinterpret_cast<EOSSDK_Presence*>(Handle);
    return pInst->GetJoinInfo(Options, OutBuffer, InOutBufferLength);
}

/**
 * To modify your own presence, you must call EOS_Presence_CreatePresenceModification to create a Presence Modification handle. To modify that handle, call
 * EOS_PresenceModification_* methods. Once you are finished, call EOS_Presence_SetPresence with your handle. You must then release your Presence Modification
 * handle by calling EOS_PresenceModification_Release.
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetStatusOptions* Options)
{
    auto pInst = reinterpret_cast<EOSSDK_PresenceModification*>(Handle);
    return pInst->SetStatus(Options);
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
    auto pInst = reinterpret_cast<EOSSDK_PresenceModification*>(Handle);
    return pInst->SetRawRichText(Options);
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
    auto pInst = reinterpret_cast<EOSSDK_PresenceModification*>(Handle);
    return pInst->SetData(Options);
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
    auto pInst = reinterpret_cast<EOSSDK_PresenceModification*>(Handle);
    return pInst->DeleteData(Options);
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
    auto pInst = reinterpret_cast<EOSSDK_PresenceModification*>(Handle);
    return pInst->SetJoinInfo(Options);
}

/**
 * Release the memory associated with an EOS_Presence_Info structure and its sub-objects. This must be called on data retrieved from EOS_Presence_CopyPresence.
 * This can be safely called on a NULL presence info object.
 *
 * @param PresenceInfo The presence info structure to be release
 */
EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo)
{
    if (PresenceInfo == nullptr)
        return;

    delete[] PresenceInfo->Platform;
    delete[] PresenceInfo->ProductId;
    delete[] PresenceInfo->ProductName;
    delete[] PresenceInfo->ProductVersion;
    delete[] PresenceInfo->RichText;
    for (int i = 0; i < PresenceInfo->RecordsCount; ++i)
    {
        delete[] PresenceInfo->Records[i].Key;
        delete[] PresenceInfo->Records[i].Value;
    }
    delete[] PresenceInfo->Records;
    delete PresenceInfo->UserId;
    delete PresenceInfo;
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
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_PresenceModification*>(PresenceModificationHandle);
    delete pInst;
}