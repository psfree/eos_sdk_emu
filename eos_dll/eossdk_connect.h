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
    struct user_state_t
    {
        bool connected;    // Peer is connected
        bool authentified; // We have peer infos
        std::chrono::steady_clock::time_point last_infos;
        Connect_Infos_pb infos;
    };

    class EOSSDK_Connect :
        public IRunFrame
    {
        static constexpr std::chrono::milliseconds user_infos_rate = std::chrono::milliseconds(3000);

        std::string _device_id;

    public:
        std::string _username; // This is used for leaderboards thing ?

        nlohmann::fifo_map<EOS_ProductUserId, user_state_t> _users;

        EOSSDK_Connect();
        ~EOSSDK_Connect();

        inline typename decltype(EOSSDK_Connect::_users)::iterator get_myself()
        {
            return _users.begin();
        }
        inline typename decltype(EOSSDK_Connect::_users)::iterator get_all_users()
        {
            return _users.begin();
        }
        inline typename decltype(EOSSDK_Connect::_users)::iterator get_other_users()
        {
            return ++(_users.begin());
        }
        inline typename decltype(EOSSDK_Connect::_users)::iterator get_user_by_userid(EOS_EpicAccountId userid)
        {
            return std::find_if(_users.begin(), _users.end(), [&userid](std::pair<EOS_ProductUserId const, user_state_t>& user)
            {
                return user.second.infos.userid() == userid->to_string();
            });
        }
        inline typename decltype(EOSSDK_Connect::_users)::iterator get_user_by_productid(EOS_ProductUserId productid)
        {
            return _users.find(productid);
        }
        inline typename decltype(EOSSDK_Connect::_users)::iterator get_user_by_name(std::string const& username)
        {
            return std::find_if(_users.begin(), _users.end(), [&username](std::pair<EOS_ProductUserId const, user_state_t>& user)
            {
                return user.second.infos.displayname() == username;
            });
        }
        inline typename decltype(EOSSDK_Connect::_users)::iterator get_end_users()
        {
            return _users.end();
        }

        //void add_session(EOS_ProductUserId session_id, std::string const& session_name);
        //void remove_session(EOS_ProductUserId session_id, std::string const& session_name);

        // Send Network messages
        bool send_connect_infos_request(Network::peer_t const& peerid, Connect_Request_Info_pb* req);
        bool send_connect_infos(Network::peer_t const& peerid, Connect_Infos_pb* infos);

        // Receive Network messages
        bool on_peer_connect(Network_Message_pb const& msg, Network_Peer_Connect_pb const& peer);
        bool on_peer_disconnect(Network_Message_pb const& msg, Network_Peer_Disconnect_pb const& peer);
        bool on_connect_infos_request(Network_Message_pb const& msg, Connect_Request_Info_pb const& req);
        bool on_connect_infos(Network_Message_pb const& msg, Connect_Infos_pb const& infos);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void Login(const EOS_Connect_LoginOptions* Options, void* ClientData, const EOS_Connect_OnLoginCallback CompletionDelegate);
        void CreateUser(const EOS_Connect_CreateUserOptions* Options, void* ClientData, const EOS_Connect_OnCreateUserCallback CompletionDelegate);
        void LinkAccount(const EOS_Connect_LinkAccountOptions* Options, void* ClientData, const EOS_Connect_OnLinkAccountCallback CompletionDelegate);
        void CreateDeviceId(const EOS_Connect_CreateDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnCreateDeviceIdCallback CompletionDelegate);
        void DeleteDeviceId(const EOS_Connect_DeleteDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnDeleteDeviceIdCallback CompletionDelegate);
        void QueryExternalAccountMappings(const EOS_Connect_QueryExternalAccountMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryExternalAccountMappingsCallback CompletionDelegate);
        void QueryProductUserIdMappings(const EOS_Connect_QueryProductUserIdMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryProductUserIdMappingsCallback CompletionDelegate);
        EOS_ProductUserId GetExternalAccountMapping(const EOS_Connect_GetExternalAccountMappingsOptions* Options);
        EOS_EResult GetProductUserIdMapping(const EOS_Connect_GetProductUserIdMappingOptions* Options, char* OutBuffer, int32_t* InOutBufferLength);
        int32_t GetLoggedInUsersCount();
        EOS_ProductUserId GetLoggedInUserByIndex(int32_t Index);
        EOS_ELoginStatus GetLoginStatus(EOS_ProductUserId LocalUserId);
        EOS_NotificationId AddNotifyAuthExpiration(const EOS_Connect_AddNotifyAuthExpirationOptions* Options, void* ClientData, const EOS_Connect_OnAuthExpirationCallback Notification);
        void RemoveNotifyAuthExpiration(EOS_NotificationId InId);
        EOS_NotificationId AddNotifyLoginStatusChanged(const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Connect_OnLoginStatusChangedCallback Notification);
        void RemoveNotifyLoginStatusChanged(EOS_NotificationId InId);
        uint32_t GetProductUserExternalAccountCount(const EOS_Connect_GetProductUserExternalAccountCountOptions * Options);
        EOS_EResult CopyProductUserExternalAccountByIndex(const EOS_Connect_CopyProductUserExternalAccountByIndexOptions * Options, EOS_Connect_ExternalAccountInfo * *OutExternalAccountInfo);
        EOS_EResult CopyProductUserExternalAccountByAccountType(const EOS_Connect_CopyProductUserExternalAccountByAccountTypeOptions * Options, EOS_Connect_ExternalAccountInfo * *OutExternalAccountInfo);
        EOS_EResult CopyProductUserExternalAccountByAccountId(const EOS_Connect_CopyProductUserExternalAccountByAccountIdOptions * Options, EOS_Connect_ExternalAccountInfo * *OutExternalAccountInfo);
        EOS_EResult CopyProductUserInfo(const EOS_Connect_CopyProductUserInfoOptions * Options, EOS_Connect_ExternalAccountInfo * *OutExternalAccountInfo);
    };
}