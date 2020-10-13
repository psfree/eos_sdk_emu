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

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthToken();
EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenOld(EOS_HAuth Handle, EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenNew(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChanged();
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedOld(EOS_HAuth Handle, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedNew(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);

namespace sdk
{
    class EOSSDK_Auth :
        public IRunCallback,
        public IRunNetwork
    {
        bool _logged_in;
        std::string _access_token;
        std::chrono::system_clock::time_point _access_expires;
        std::string _refresh_token;
        std::chrono::system_clock::time_point _refresh_expires;

    public:
        EOSSDK_Auth();
        ~EOSSDK_Auth();

        virtual bool CBRunFrame();
        virtual bool RunNetwork(Network_Message_pb const& msg);
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void Login(const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate);
        void Logout(const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate);
        void LinkAccount(const EOS_Auth_LinkAccountOptions* Options, void* ClientData, const EOS_Auth_OnLinkAccountCallback CompletionDelegate);
        void DeletePersistentAuth(const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate);
        void VerifyUserAuth(const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate);
        int32_t GetLoggedInAccountsCount();
        EOS_EpicAccountId GetLoggedInAccountByIndex(int32_t Index);
        EOS_ELoginStatus GetLoginStatus(EOS_EpicAccountId LocalUserId);
        EOS_EResult CopyUserAuthTokenOld(EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
        EOS_EResult CopyUserAuthToken(const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken);
        EOS_NotificationId AddNotifyLoginStatusChangedOld(void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);
        EOS_NotificationId AddNotifyLoginStatusChanged(const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);
        void RemoveNotifyLoginStatusChanged(EOS_NotificationId InId);
    };
}