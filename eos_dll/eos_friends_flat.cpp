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

#include "eossdk_friends.h"

using namespace sdk;

EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    pInst->QueryFriends(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    pInst->SendInvite(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    pInst->AcceptInvite(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    pInst->RejectInvite(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const EOS_Friends_GetFriendsCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    return pInst->GetFriendsCount(Options);
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const EOS_Friends_GetFriendAtIndexOptions* Options)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    return pInst->GetFriendAtIndex(Options);
}

EOS_DECLARE_FUNC(EOS_EFriendsStatus) EOS_Friends_GetStatus(EOS_HFriends Handle, const EOS_Friends_GetStatusOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EFriendsStatus::EOS_FS_NotFriends;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    return pInst->GetStatus(Options);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    return pInst->AddNotifyFriendsUpdate(Options, ClientData, FriendsUpdateHandler);
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Friends*>(Handle);
    pInst->RemoveNotifyFriendsUpdate(NotificationId);
}
