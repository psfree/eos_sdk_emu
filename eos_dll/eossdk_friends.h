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
#include "network.h"

namespace sdk
{
    class EOSSDK_Friends :
        public IRunCallback,
        public IRunNetwork
    {
        nlohmann::fifo_map<EOS_EpicAccountId, Friend_Info_pb> _friends;

    public:
        EOSSDK_Friends();
        ~EOSSDK_Friends();

        // Send Network messages
        bool send_friend_info_request(Network::peer_t const& peerid, Friend_Info_Request_pb* req);
        bool send_friend_info(Network::peer_t const& peerid, Friend_Info_pb* infos);
        //bool send_friend_invite(Network::peer_t const& peerid, Friend_Invite_pb* invite) const;
        //bool send_friend_invite_response(Network::peer_t const& peerid, Friend_Invite_Response_pb* resp) const;

        // Receive Network messages
        bool on_friend_info_request(Network_Message_pb const& msg, Friend_Info_Request_pb const& req);
        bool on_friend_info(Network_Message_pb const& msg, Friend_Info_pb const& infos);
        //bool on_friend_invite(Network_Message_pb const& msg, Friend_Invite_pb const& invite);
        //bool on_friend_invite_response(Network_Message_pb const& msg, Friend_Invite_Response_pb const& resp);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void QueryFriends(const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate);
        void SendInvite(const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate);
        void AcceptInvite(const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate);
        void RejectInvite(const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate);
        int32_t GetFriendsCount(const EOS_Friends_GetFriendsCountOptions* Options);
        EOS_EpicAccountId GetFriendAtIndex(const EOS_Friends_GetFriendAtIndexOptions* Options);
        EOS_EFriendsStatus GetStatus(const EOS_Friends_GetStatusOptions* Options);
        EOS_NotificationId AddNotifyFriendsUpdate(const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler);
        void RemoveNotifyFriendsUpdate(EOS_NotificationId NotificationId);
    };
}
