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

namespace sdk
{
    class EOSSDK_Connect :
        public IRunFrame
    {
    public:
        EOS_ProductUserIdDetails _productid;
        std::string _username; // This is used for leaderboards thing ?

        EOSSDK_Connect();
        ~EOSSDK_Connect();

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
    };
}