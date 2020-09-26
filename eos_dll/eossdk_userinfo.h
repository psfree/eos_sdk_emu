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
    class EOSSDK_UserInfo :
        public IRunCallback,
        public IRunNetwork
    {
        static constexpr auto userinfo_query_timeout = std::chrono::milliseconds(20000);

        std::unordered_map<EOS_EpicAccountId, UserInfo_Info_pb> _userinfos;
        std::unordered_map<EOS_EpicAccountId, std::list<pFrameResult_t>> _userinfos_queries;

    public:
        EOSSDK_UserInfo();
        ~EOSSDK_UserInfo();

        void setup_myself();
        UserInfo_Info_pb& get_myself();
        UserInfo_Info_pb* get_userinfo(EOS_EpicAccountId userid);

        // Send Network messages
        bool send_userinfo_request(Network::peer_t const& peerid, UserInfo_Info_Request_pb* req);
        bool send_my_userinfo(Network::peer_t const& peerid);

        // Receive Network messages
        bool on_userinfo_request(Network_Message_pb const& msg, UserInfo_Info_Request_pb const& req);
        bool on_userinfo(Network_Message_pb const& msg, UserInfo_Info_pb const& infos);

        void QueryUserInfo(const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate);
        void QueryUserInfoByDisplayName(const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate);
        void QueryUserInfoByExternalAccount(const EOS_UserInfo_QueryUserInfoByExternalAccountOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByExternalAccountCallback CompletionDelegate);
        EOS_EResult CopyUserInfo(const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo** OutUserInfo);
        uint32_t GetExternalUserInfoCount(const EOS_UserInfo_GetExternalUserInfoCountOptions* Options);
        EOS_EResult CopyExternalUserInfoByIndex(const EOS_UserInfo_CopyExternalUserInfoByIndexOptions* Options, EOS_UserInfo_ExternalUserInfo** OutExternalUserInfo);
        EOS_EResult CopyExternalUserInfoByAccountType(const EOS_UserInfo_CopyExternalUserInfoByAccountTypeOptions* Options, EOS_UserInfo_ExternalUserInfo** OutExternalUserInfo);
        EOS_EResult CopyExternalUserInfoByAccountId(const EOS_UserInfo_CopyExternalUserInfoByAccountIdOptions* Options, EOS_UserInfo_ExternalUserInfo** OutExternalUserInfo);

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);
    };
}
