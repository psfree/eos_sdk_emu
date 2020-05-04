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

#include "eos_friends.h"

using namespace sdk;

/**
 * The Friends Interface is used to manage a user's friends list, by interacting with the backend services, and to retrieve the cached list of friends and pending invitations.
 * All Friends Interface calls take a handle of type EOS_HFriends as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetFriendsInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetFriendsInterface
 */

 /**
  * Starts an asynchronous task that reads the user's friends list from the backend service, caching it for future use.
  *
  * @param Options structure containing the account for which to retrieve the friends list
  * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
  */
EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
}

/**
 * Starts an asynchronous task that sends a friend invitation to another user. The completion delegate is executed after the backend response has been received.
 * It does not indicate that the target user has responded to the friend invitation.
 *
 * @param Options structure containing the account to send the invite from and the account to send the invite to
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
}

/**
 * Starts an asynchronous task that accepts a friend invitation from another user. The completion delegate is executed after the backend response has been received.
 *
 * @param Options structure containing the logged in account and the inviting account
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
}

/**
 * Starts an asynchronous task that rejects a friend invitation from another user. The completion delegate is executed after the backend response has been received.
 *
 * @param Options structure containing the logged in account and the inviting account
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
}

/**
 * Retrieves the number of friends on the friends list that has already been retrieved by the EOS_Friends_QueryFriends API.
 *
 * @param Options structure containing the account id of the friends list
 * @return the number of friends on the list
 *
 * @see EOS_Friends_GetFriendAtIndex
 */
EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const EOS_Friends_GetFriendsCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
    return 0;
}

/**
 * Retrieves the account id of an entry from the friends list that has already been retrieved by the EOS_Friends_QueryFriends API.
 * The account id returned by this function may belong to an account that has been invited to be a friend or that has invited the local user to be a friend.
 * To determine if the account id returned by this function is a friend or a pending friend invitation, use the EOS_Friends_GetStatus function.
 *
 * @param Options structure containing the account id of the friends list and the index into the list
 * @return the account id of the friend. Note that if the index provided is out of bounds, the returned account id will be a "null" account id.
 *
 * @see EOS_Friends_GetFriendsCount
 * @see EOS_Friends_GetStatus
 */
EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const EOS_Friends_GetFriendAtIndexOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
    return nullptr;
}

/**
 * Retrieve the friendship status between the local user and another user.
 *
 * @param Options structure containing the account id of the friend list to check and the account of the user to test friendship status
 * @return A value indicating whether the two accounts have a friendship, pending invites in either direction, or no relationship
 *         EOS_FS_Friends is returned for two users that have confirmed friendship
 *         EOS_FS_InviteSent is returned when the local user has sent a friend invitation but the other user has not accepted or rejected it
 *         EOS_FS_InviteReceived is returned when the other user has sent a friend invitation to the local user
 *         EOS_FS_NotFriends is returned when there is no known relationship
 *
 * @see EOS_EFriendsStatus
 */
EOS_DECLARE_FUNC(EOS_EFriendsStatus) EOS_Friends_GetStatus(EOS_HFriends Handle, const EOS_Friends_GetStatusOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
    return EOS_EFriendsStatus::EOS_FS_Friends;
}

/**
 * Listen for changes to friends for a particular account.
 *
 * @param Options Information about who would like notifications.
 * @param ClientData This value is returned to the caller when FriendsUpdateHandler is invoked.
 * @param FriendsUpdateHandler The callback to be invoked when a change to any friend status changes.
 * @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
    return 0;
}

/**
 * Stop listening for friends changes on a previously bound handler.
 *
 * @param NotificationId The previously bound notification ID.
 */
EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Friends*>(Handle);
}
