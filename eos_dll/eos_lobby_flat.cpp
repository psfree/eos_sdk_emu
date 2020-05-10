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

#include "eossdk_lobby.h"

using namespace sdk;

/**
 * The Lobby Interface is used to manage lobbies that provide a persistent connection between users and
 * notifications of data sharing/updates.  Lobbies may also be found by advertising and searching with the backend service.
 * All Lobby Interface calls take a handle of type EOS_HLobby as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetLobbyInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetLobbyInterface
 */

 /**
  * Creates a lobby and adds the user to the lobby membership.  There is no data associated with the lobby at the start and can be added vis EOS_Lobby_UpdateLobbyModification
  *
  * @param Options Required fields for the creation of a lobby such as a user count and its starting advertised state
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate A callback that is fired when the create operation completes, either successfully or in error
  *
  * @return EOS_Success if the creation completes successfully
  *         EOS_InvalidParameters if any of the options are incorrect
  *         EOS_LimitExceeded if the number of allowed lobbies is exceeded
  */
EOS_DECLARE_FUNC(void) EOS_Lobby_CreateLobby(EOS_HLobby Handle, const EOS_Lobby_CreateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnCreateLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Destroy a lobby given a lobby id
 *
 * @param Options Structure containing information about the lobby to be destroyed
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the destroy operation completes, either successfully or in error
 *
 * @return EOS_Success if the destroy completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_AlreadyPending if the lobby is already marked for destroy
 *         EOS_NotFound if the lobby to be destroyed does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_DestroyLobby(EOS_HLobby Handle, const EOS_Lobby_DestroyLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnDestroyLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Join a lobby, creating a local instance under a given lobby id.  Backend will validate various conditions to make sure it is possible to join the lobby.
 *
 * @param Options Structure containing information about the lobby to be joined
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the join operation completes, either successfully or in error
 *
 * @return EOS_Success if the destroy completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobby(EOS_HLobby Handle, const EOS_Lobby_JoinLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Leave a lobby given a lobby id
 *
 * @param Options Structure containing information about the lobby to be left
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the leave operation completes, either successfully or in error
 *
 * @return EOS_Success if the leave completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_AlreadyPending if the lobby is already marked for leave
 *         EOS_NotFound if a lobby to be left does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveLobby(EOS_HLobby Handle, const EOS_Lobby_LeaveLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Creates a lobby modification handle (EOS_HLobbyModification). The lobby modification handle is used to modify an existing lobby and can be applied with EOS_Lobby_UpdateLobby.
 * The EOS_HLobbyModification must be released by calling EOS_LobbyModification_Release once it is no longer needed.
 *
 * @param Options Required fields such as lobby id
 * @param OutLobbyModificationHandle Pointer to a Lobby Modification Handle only set if successful
 * @return EOS_Success if we successfully created the Lobby Modification Handle pointed at in OutLobbyModificationHandle, or an error result if the input data was invalid
 *		   EOS_InvalidParameters if any of the options are incorrect
 *
 * @see EOS_LobbyModification_Release
 * @see EOS_Lobby_UpdateLobby
 * @see EOS_LobbyModification_*
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_UpdateLobbyModification(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyModificationOptions* Options, EOS_HLobbyModification* OutLobbyModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Update a lobby given a lobby modification handle created via EOS_Lobby_UpdateLobbyModification
 *
 * @param Options Structure containing information about the lobby to be updated
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the update operation completes, either successfully or in error
 *
 * @return EOS_Success if the update completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Lobby_NotOwner if the lobby modification contains modifications that are only allowed by the owner
 *         EOS_NotFound if the lobby to update does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_UpdateLobby(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnUpdateLobbyCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Promote an existing member of the lobby to owner, allowing them to make lobby data modifications
 *
 * @param Options Structure containing information about the lobby and member to be promoted
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the promotion operation completes, either successfully or in error
 *
 * @return EOS_Success if the promote completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Lobby_NotOwner if the calling user is not the owner of the lobby
 *         EOS_NotFound if the lobby of interest does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_PromoteMember(EOS_HLobby Handle, const EOS_Lobby_PromoteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnPromoteMemberCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Kick an existing member from the lobby
 *
 * @param Options Structure containing information about the lobby and member to be kicked
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the kick operation completes, either successfully or in error
 *
 * @return EOS_Success if the kick completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Lobby_NotOwner if the calling user is not the owner of the lobby
 *         EOS_NotFound if a lobby of interest does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_KickMember(EOS_HLobby Handle, const EOS_Lobby_KickMemberOptions* Options, void* ClientData, const EOS_Lobby_OnKickMemberCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Register to receive notifications when a lobby owner updates the attributes associated with the lobby.
 * @note must call RemoveNotifyLobbyUpdateReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyUpdateReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * Unregister from receiving notifications when a lobby changes its data.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Register to receive notifications when a lobby member updates the attributes associated with themselves inside the lobby.
 * @note must call RemoveNotifyLobbyMemberUpdateReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberUpdateReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * Unregister from receiving notifications when lobby members change their data.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Register to receive notifications about the changing status of lobby members.
 * @note must call RemoveNotifyLobbyMemberStatusReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberStatusReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * Unregister from receiving notifications when lobby members status change.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Send an invite to another user.  User must be a member of the lobby or else the call will fail
 *
 * @param Options Structure containing information about the lobby and user to invite
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the send invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the send invite completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the lobby to send the invite from does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_SendInvite(EOS_HLobby Handle, const EOS_Lobby_SendInviteOptions* Options, void* ClientData, const EOS_Lobby_OnSendInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Reject an invite from another user.
 *
 * @param Options Structure containing information about the invite to reject
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the reject invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the invite rejection completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_RejectInvite(EOS_HLobby Handle, const EOS_Lobby_RejectInviteOptions* Options, void* ClientData, const EOS_Lobby_OnRejectInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Retrieve all existing invites for a single user
 *
 * @param Options Structure containing information about the invites to query
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the query invites operation completes, either successfully or in error
 *
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_QueryInvites(EOS_HLobby Handle, const EOS_Lobby_QueryInvitesOptions* Options, void* ClientData, const EOS_Lobby_OnQueryInvitesCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Get the number of known invites for a given user
 *
 * @param Options the Options associated with retrieving the current invite count
 *
 * @return number of known invites for a given user or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Lobby_GetInviteCount(EOS_HLobby Handle, const EOS_Lobby_GetInviteCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * Retrieve an invite id from a list of active invites for a given user
 *
 * @param Options Structure containing the input parameters
 *
 * @return EOS_Success if the input is valid and an invite id was returned
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite doesn't exist
 *
 * @see EOS_Lobby_GetInviteCount
 * @see EOS_Lobby_CopyLobbyDetailsHandleByInviteId
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetInviteIdByIndex(EOS_HLobby Handle, const EOS_Lobby_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Create a lobby search handle.  This handle may be modified to include various search parameters.
 * Searching is possible in three methods, all mutually exclusive
 * - set the lobby id to find a specific lobby
 * - set the target user id to find a specific user
 * - set lobby parameters to find an array of lobbies that match the search criteria (not available yet)
 *
 * @param Options Structure containing required parameters such as the maximum number of search results
 * @param OutLobbySearchHandle The new search handle or null if there was an error creating the search handle
 *
 * @return EOS_Success if the search creation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CreateLobbySearch(EOS_HLobby Handle, const EOS_Lobby_CreateLobbySearchOptions* Options, EOS_HLobbySearch* OutLobbySearchHandle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Register to receive notifications about lobby invites sent to local users.
 * @note must call RemoveNotifyLobbyInviteReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteReceivedCallback NotificationFn)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * Unregister from receiving notifications when a user receives a lobby invitation.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteReceived(EOS_HLobby Handle, EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * EOS_Lobby_CopyLobbyDetailsHandleByInviteId is used to immediately retrieve a handle to the lobby information from after notification of an invite
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutLobbyDetailsHandle out parameter used to receive the lobby handle
 *
 * @return EOS_Success if the information is available and passed out in OutLobbyDetailsHandle
 *         EOS_InvalidParameters if you pass an invalid invite id or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound If the invite id cannot be found
 *
 * @see EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions
 * @see EOS_LobbyDetails_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByInviteId(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Create a handle to an existing lobby.
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing information about the lobby to retrieve
 * @param OutLobbyDetailsHandle The new active lobby handle or null if there was an error
 *
 * @return EOS_Success if the lobby handle was created successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the lobby doesn't exist
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandle(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * To modify lobbies or the lobby member data, you must call EOS_Lobby_UpdateLobbyModification to create a Lobby Modification handle. To modify that handle, call
 * EOS_LobbyModification_* methods. Once you are finished, call EOS_Lobby_UpdateLobby with your handle. You must then release your Lobby Modification
 * handle by calling EOS_LobbyModification_Release.
 */

 /**
  * Set the permissions associated with this lobby.
  * The permissions range from "public" to "invite only" and are described by EOS_ELobbyPermissionLevel
  *
  * @param Options Options associated with the permission level of the lobby
  *
  * @return EOS_Success if setting this parameter was successful
  *         EOS_IncompatibleVersion if the API version passed in is incorrect
  */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetPermissionLevel(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetPermissionLevelOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Set the maximum number of members allowed in this lobby.
 * When updating the lobby, it is not possible to reduce this number below the current number of existing members
 *
 * @param Options Options associated with max number of members in this lobby
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetMaxMembers(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetMaxMembersOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Associate an attribute with this lobby
 * An attribute is something may be public or private with the lobby.
 * If public, it can be queried for in a search, otherwise the data remains known only to lobby members
 *
 * @param Options Options to set the attribute and its visibility state
 *
 * @return EOS_Success if setting this parameter was successful
 *		   EOS_InvalidParameters if the attribute is missing information or otherwise invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Remove an attribute associated with the lobby
 *
 * @param Options Specify the key of the attribute to remove
 *
 * @return EOS_Success if removing this parameter was successful
 *		   EOS_InvalidParameters if the key is null or empty
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Associate an attribute with a member of the lobby
 * Lobby member data is always private to the lobby
 *
 * @param Options Options to set the attribute and its visibility state
 *
 * @return EOS_Success if setting this parameter was successful
 *		   EOS_InvalidParameters if the attribute is missing information or otherwise invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddMemberAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Remove an attribute associated with of member of the lobby
 *
 * @param Options Specify the key of the member attribute to remove
 *
 * @return EOS_Success if removing this parameter was successful
 *		   EOS_InvalidParameters if the key is null or empty
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveMemberAttributeOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * A "read only" representation of an existing lobby that games interact with externally.
 * Both the lobby and lobby search interfaces interface use this common class for lobby management and search results
 */

 /**
  * Get the product user id of the current owner for a given lobby
  *
  * @param Options Structure containing the input parameters
  *
  * @return the product user id for the lobby owner or null if the input parameters are invalid
  */
EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetLobbyOwner(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetLobbyOwnerOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return nullptr;
}

/**
 * EOS_LobbyDetails_CopyInfo is used to immediately retrieve a copy of lobby information from a given source such as a existing lobby or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsInfo, must be passed to EOS_LobbyDetails_Info_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutLobbyDetailsInfo Out parameter used to receive the EOS_LobbyDetails_Info structure.
 *
 * @return EOS_Success if the information is available and passed out in OutLobbyDetailsInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_LobbyDetails_Info
 * @see EOS_LobbyDetails_CopyInfoOptions
 * @see EOS_LobbyDetails_Info_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyInfoOptions* Options, EOS_LobbyDetails_Info** OutLobbyDetailsInfo)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of attributes associated with this lobby
 *
 * @param Options the Options associated with retrieving the attribute count
 *
 * @return number of attributes on the lobby or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetAttributeCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * EOS_LobbyDetails_CopyAttributeByIndex is used to immediately retrieve a copy of a lobby attribute from a given source such as a existing lobby or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyAttributeByIndexOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * EOS_LobbyDetails_CopyAttributeByKey is used to immediately retrieve a copy of a lobby attribute from a given source such as a existing lobby or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyAttributeByKeyOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of members associated with this lobby
 *
 * @param Options the Options associated with retrieving the member count
 *
 * @return number of members in the existing lobby or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * EOS_LobbyDetails_GetMemberByIndex is used to immediately retrieve individual members registered with a lobby.
 *
 * @param Options Structure containing the input parameters
 *
 * @return the product user id for the registered member at a given index or null if that index is invalid
 *
 * @see EOS_LobbyDetails_GetMemberCount
 * @see EOS_LobbyDetails_GetMemberByIndexOptions
 */
EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetMemberByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberByIndexOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return nullptr;
}

/**
 * EOS_LobbyDetails_GetMemberAttributeCount is used to immediately retrieve the attribute count for members in a lobby.
 *
 * @param Options Structure containing the input parameters
 *
 * @return the number of attributes associated with a given lobby member or 0 if that member is invalid
 *
 * @see EOS_LobbyDetails_GetMemberCount
 * @see EOS_LobbyDetails_GetMemberAttributeCountOptions
 */
EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberAttributeCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * EOS_LobbyDetails_CopyMemberAttributeByIndex is used to immediately retrieve a copy of a lobby member attribute from an existing lobby.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyMemberAttributeByIndexOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByIndexOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * EOS_LobbyDetails_CopyMemberAttributeByKey is used to immediately retrieve a copy of a lobby member attribute from an existing lobby.
 * If the call returns an EOS_Success result, the out parameter, OutAttribute, must be passed to EOS_Lobby_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutAttribute Out parameter used to receive the EOS_Lobby_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Lobby_Attribute
 * @see EOS_LobbyDetails_CopyMemberAttributeByKeyOptions
 * @see EOS_Lobby_Attribute_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByKeyOptions* Options, EOS_Lobby_Attribute** OutAttribute)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Class responsible for the creation, setup, and execution of a search query.
 * Search parameters are defined, the query is executed and the search results are returned within this object
 */

 /**
  * Find lobbies matching the search criteria setup via this lobby search handle.
  * When the operation completes, this handle will have the search results that can be parsed
  *
  * @param Options Structure containing information about the search criteria to use
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate A callback that is fired when the search operation completes, either successfully or in error
  *
  * @return EOS_Success if the find operation completes successfully
  *         EOS_NotFound if searching for an individual lobby by lobby id or target user id returns no results
  *         EOS_InvalidParameters if any of the options are incorrect
  */
EOS_DECLARE_FUNC(void) EOS_LobbySearch_Find(EOS_HLobbySearch Handle, const EOS_LobbySearch_FindOptions* Options, void* ClientData, const EOS_LobbySearch_OnFindCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
}

/**
 * Set a lobby id to find and will return at most one search result.  Setting TargetUserId or SearchParameters will result in EOS_LobbySearch_Find failing
 *
 * @param Options A specific lobby id for which to search
 *
 * @return EOS_Success if setting this lobby id was successful
 *         EOS_InvalidParameters if the lobby id is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetLobbyId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetLobbyIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Set a target user id to find.  Setting LobbyId or SearchParameters will result in EOS_LobbySearch_Find failing
 * NOTE: a search result will only be found if this user is in a public lobby
 *
 * @param Options a specific target user id to find
 *
 * @return EOS_Success if setting this target user id was successful
 *         EOS_InvalidParameters if the target user id is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetTargetUserId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetTargetUserIdOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/** NYI */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetParameterOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/** NYI */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_RemoveParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_RemoveParameterOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Set the maximum number of search results to return in the query, can't be more than EOS_LOBBY_MAX_SEARCH_RESULTS
 *
 * @param Options maximum number of search results to return in the query
 *
 * @return EOS_Success if setting the max results was successful
 *         EOS_InvalidParameters if the number of results requested is invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetMaxResults(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetMaxResultsOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of search results found by the search parameters in this search
 *
 * @param Options Options associated with the search count
 *
 * @return return the number of search results found by the query or 0 if search is not complete
 */
EOS_DECLARE_FUNC(uint32_t) EOS_LobbySearch_GetSearchResultCount(EOS_HLobbySearch Handle, const EOS_LobbySearch_GetSearchResultCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return 0;
}

/**
 * EOS_LobbySearch_CopySearchResultByIndex is used to immediately retrieve a handle to the lobby information from a given search result.
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutLobbyDetailsHandle out parameter used to receive the lobby details handle
 *
 * @return EOS_Success if the information is available and passed out in OutLobbyDetailsHandle
 *         EOS_InvalidParameters if you pass an invalid index or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_LobbySearch_CopySearchResultByIndexOptions
 * @see EOS_LobbyDetails_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_CopySearchResultByIndex(EOS_HLobbySearch Handle, const EOS_LobbySearch_CopySearchResultByIndexOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOSSDK_Lobby*>(Handle);
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_LobbyModification_Release(EOS_HLobbyModification LobbyModificationHandle)
{
    LOG(Log::LogLevel::TRACE, "");
}

/**
 * Release the memory associated with a single lobby. This must be called on data retrieved from EOS_LobbySearch_CopySearchResultByIndex.
 *
 * @param LobbyHandle - The lobby handle to release
 *
 * @see EOS_LobbySearch_CopySearchResultByIndex
 */
EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Release(EOS_HLobbyDetails LobbyHandle)
{
    LOG(Log::LogLevel::TRACE, "");
}

/**
 * Release the memory associated with a lobby search. This must be called on data retrieved from EOS_Lobby_CreateLobbySearch.
 *
 * @param LobbySearchHandle - The lobby search handle to release
 *
 * @see EOS_Lobby_CreateLobbySearch
 */
EOS_DECLARE_FUNC(void) EOS_LobbySearch_Release(EOS_HLobbySearch LobbySearchHandle)
{
    LOG(Log::LogLevel::TRACE, "");
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Info_Release(EOS_LobbyDetails_Info* LobbyDetailsInfo)
{
    LOG(Log::LogLevel::TRACE, "");
}

EOS_DECLARE_FUNC(void) EOS_Lobby_Attribute_Release(EOS_Lobby_Attribute* LobbyAttribute)
{
    LOG(Log::LogLevel::TRACE, "");
}
