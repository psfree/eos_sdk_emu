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

#include "eossdk_auth.h"
#include "eossdk_platform.h"
#include "settings.h"

namespace sdk
{

EOSSDK_Auth::EOSSDK_Auth()
{
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_Auth::~EOSSDK_Auth()
{
    GetCB_Manager().unregister_callbacks(this);
    GetCB_Manager().remove_all_notifications(this);
}

void EOSSDK_Auth::Login(const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (Options != nullptr)
    {
        if (Options->ApiVersion >= EOS_AUTH_LOGIN_API_002)
        {
            //Options->ScopeFlags;
        }

        LOG(Log::LogLevel::DEBUG, "ApiVersion = %u", Options->ApiVersion);
        LOG(Log::LogLevel::DEBUG, "Credentials ApiVersion = %u", Options->Credentials->ApiVersion);
        LOG(Log::LogLevel::DEBUG, "Id    = '%s'", Options->Credentials->Id);
        LOG(Log::LogLevel::DEBUG, "Token = '%s'", Options->Credentials->Token);
        LOG(Log::LogLevel::DEBUG, "Type  = %u", Options->Credentials->Type);

        pFrameResult_t res(new FrameResult);

        EOS_Auth_LoginCallbackInfo &lci = res->CreateCallback<EOS_Auth_LoginCallbackInfo>((CallbackFunc)CompletionDelegate, std::chrono::milliseconds(10000));
        lci.ClientData = ClientData;
        lci.LocalUserId = &Settings::Inst().userid;
        lci.ResultCode = EOS_EResult::EOS_Success;
        lci.PinGrantInfo = nullptr;
        res->done = true;

        GetCB_Manager().add_callback(this, res);
    }
}

void EOSSDK_Auth::Logout(const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (Options != nullptr)
    {
        //if (Options->ApiVersion >= EOS_AUTH_LOGOUT_API_001)

        pFrameResult_t res(new FrameResult);

        EOS_Auth_LogoutCallbackInfo& lci = res->CreateCallback<EOS_Auth_LogoutCallbackInfo>((CallbackFunc)CompletionDelegate);
        lci.ClientData = ClientData;
        lci.LocalUserId = &Settings::Inst().userid;
        lci.ResultCode = EOS_EResult::EOS_Success;
        res->done = true;

        GetCB_Manager().add_callback(this, res);
    }
}

void EOSSDK_Auth::DeletePersistentAuth(const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
}

void EOSSDK_Auth::VerifyUserAuth(const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
}

int32_t EOSSDK_Auth::GetLoggedInAccountsCount()
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return 1;
}

EOS_EpicAccountId EOSSDK_Auth::GetLoggedInAccountByIndex(int32_t Index)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (Index == 0)
        return &Settings::Inst().userid;

    return nullptr;
}

EOS_ELoginStatus EOSSDK_Auth::GetLoginStatus(EOS_EpicAccountId LocalUserId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (LocalUserId == &Settings::Inst().userid)
        return EOS_ELoginStatus::EOS_LS_LoggedIn;

    return EOS_ELoginStatus::EOS_LS_NotLoggedIn;
}

EOS_EResult EOSSDK_Auth::CopyUserAuthToken(const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (OutUserAuthToken == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (*LocalUserId == Settings::Inst().userid)
    {
        EOS_Auth_Token* token = new EOS_Auth_Token;
        time_t now;
        time(&now);
        
        token->ApiVersion = Options->ApiVersion;
        token->App = Settings::Inst().gamename.c_str();
        token->ClientId = GetEOS_Platform()._client_id.c_str();
        token->AccountId = LocalUserId;
        
        token->AccessToken = "ACCE22105E4";
        token->ExpiresIn = 99999999;
        token->ExpiresAt = new char[64];
        now += 99999999;
        strftime((char*)token->ExpiresAt, 64, "%FT%TZ", gmtime(&now));
        now -= 99999999;


        token->AuthType = EOS_EAuthTokenType::EOS_ATT_User;

        token->RefreshToken = "A3EF3E28105E4";
        token->RefreshExpiresIn = 999999;
        token->RefreshExpiresAt = new char[64];
        now += 999999;
        strftime((char*)token->RefreshExpiresAt, 64, "%FT%TZ", gmtime(&now));
        now -= 999999;

        *OutUserAuthToken = token;
        return EOS_EResult::EOS_Success;
    }
    
    LOG(Log::LogLevel::DEBUG, "Accountid not found: %p %s", LocalUserId, (LocalUserId == nullptr ? "" : LocalUserId->to_string().c_str()));
    return EOS_EResult::EOS_NotFound;
}

EOS_NotificationId EOSSDK_Auth::AddNotifyLoginStatusChanged(const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);

    EOS_Auth_LoginStatusChangedCallbackInfo& lscci = res->CreateCallback<EOS_Auth_LoginStatusChangedCallbackInfo>((CallbackFunc)Notification);
    lscci.ClientData = ClientData;
    lscci.CurrentStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;
    lscci.PrevStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;
    lscci.LocalUserId = &Settings::Inst().userid;

    return GetCB_Manager().add_notification(this, res);
}

void EOSSDK_Auth::RemoveNotifyLoginStatusChanged(EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    GetCB_Manager().remove_notification(this, InId);
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Auth::CBRunFrame()
{
    return false;
}

bool EOSSDK_Auth::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_Auth::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Auth::FreeCallback(pFrameResult_t res)
{

}

}