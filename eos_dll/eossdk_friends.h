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
#include "network.h"

namespace sdk
{
    struct friend_infos_t
    {
        bool online;
        std::chrono::steady_clock::time_point last_hearbeat;
        std::chrono::steady_clock::time_point last_infos;
        Friend_Info_pb infos;
    };

    class EOSSDK_Friends :
        public IRunFrame
    {
        // Heartbeat rate
        static constexpr auto alive_heartbeat_rate = std::chrono::seconds(2);
        // Heartbeat timeout to delete disconnected friends
        static constexpr auto alive_heartbeat = std::chrono::seconds(10);
        // Retrieve all friends infos rate (except rich_presence)
        static constexpr auto friend_infos_rate = std::chrono::seconds(5);

        std::chrono::steady_clock::time_point _last_heartbeat;

        nlohmann::fifo_map<std::string, friend_infos_t> _friends;
        nlohmann::fifo_map<std::string, friend_infos_t> _friends_cache_for_query;

    public:
        EOSSDK_Friends();
        ~EOSSDK_Friends();

        friend_infos_t* get_friend(std::string const& userid);
        std::pair<std::string const, friend_infos_t>* get_friend_by_name(std::string const& username);

        // Send Network messages
        bool send_heartbeat(Friend_Heartbeat_pb *hb);
        bool send_friend_info_request(Network::peer_t const& peerid, Friend_Info_Request_pb* req);
        bool send_friend_info(Network::peer_t const& peerid, Friend_Info_pb* infos);

        // Receive Network messages
        bool on_heartbeat(Network_Message_pb const& msg, Friend_Heartbeat_pb const& hb);
        bool on_friend_info_request(Network_Message_pb const& msg, Friend_Info_Request_pb const& req);
        bool on_friend_info(Network_Message_pb const& msg, Friend_Info_pb const& infos);

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
