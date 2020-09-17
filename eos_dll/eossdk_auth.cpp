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
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

EOSSDK_Auth::EOSSDK_Auth():
    _logged_in(false)
{
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_Auth::~EOSSDK_Auth()
{
    GetCB_Manager().unregister_callbacks(this);
    GetCB_Manager().remove_all_notifications(this);
}

/**
 * The Auth Interface is used to manage local user permissions and access to backend services through the verification of various forms of credentials.
 * All Auth Interface calls take a handle of type EOS_HAuth as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetAuthInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetAuthInterface
 */

 /**
  * Login/Authenticate with user credentials.
  *
  * @param Options structure containing the account credentials to use during the login operation
  * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate a callback that is fired when the login operation completes, either successfully or in error
  */
void EOSSDK_Auth::Login(const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Auth_LoginCallbackInfo& lci = res->CreateCallback<EOS_Auth_LoginCallbackInfo>((CallbackFunc)CompletionDelegate, std::chrono::milliseconds(1000));
    lci.ClientData = ClientData;
    lci.LocalUserId = Settings::Inst().userid;

    if (Options == nullptr)
    {
        lci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        APP_LOG(Log::LogLevel::DEBUG, "ApiVersion = %u", Options->ApiVersion);
        switch (Options->ApiVersion)
        {
            case EOS_AUTH_LOGIN_API_002:
            {
                //Options->ScopeFlags;
            }

            case EOS_AUTH_LOGIN_API_001:
            {
                APP_LOG(Log::LogLevel::DEBUG, "Credentials ApiVersion = %u", Options->Credentials->ApiVersion);
                switch (Options->Credentials->ApiVersion)
                {
                    case EOS_AUTH_CREDENTIALS_API_003:
                    {
                        auto* v = reinterpret_cast<const EOS_Auth_Credentials003*>(Options->Credentials);
                        APP_LOG(Log::LogLevel::DEBUG, "SystemAuthCredentialsOptions = %p", v->SystemAuthCredentialsOptions);
                        APP_LOG(Log::LogLevel::DEBUG, "ExternalType                 = %d", v->ExternalType);
                    }
                    case EOS_AUTH_CREDENTIALS_API_002:
                    {
                        auto* v = reinterpret_cast<const EOS_Auth_Credentials002*>(Options->Credentials);
                    }
                    case EOS_AUTH_CREDENTIALS_API_001:
                    {
                        auto* v = reinterpret_cast<const EOS_Auth_Credentials001*>(Options->Credentials);
                        APP_LOG(Log::LogLevel::DEBUG, "Id                           = %s", v->Id);
                        APP_LOG(Log::LogLevel::DEBUG, "Token                        = %s", v->Token);
                        APP_LOG(Log::LogLevel::DEBUG, "Type                         = %u", v->Type);
                    }
                }
            }
        }

        lci.ResultCode = EOS_EResult::EOS_Success;
        lci.PinGrantInfo = nullptr;

        _logged_in = true;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Signs the player out of the online service.
 *
 * @param Options structure containing information about which account to log out.
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the logout operation completes, either successfully or in error
 */
void EOSSDK_Auth::Logout(const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Auth_LogoutCallbackInfo& lci = res->CreateCallback<EOS_Auth_LogoutCallbackInfo>((CallbackFunc)CompletionDelegate);
    lci.ClientData = ClientData;
    lci.LocalUserId = Settings::Inst().userid;

    if (Options == nullptr)
    {
        lci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        lci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Link external account by continuing previous login attempt with a continuance token.
 *
 * On Desktop and Mobile platforms, the user will be presented the Epic Account Portal to resolve their identity.
 *
 * On Console, the user will login to their Epic Account using an external device, e.g. a mobile device or a desktop PC,
 * by browsing to the presented authentication URL and entering the device code presented by the game on the console.
 *
 * On success, the user will be logged in at the completion of this action.
 * This will commit this external account to the Epic Account and cannot be undone in the SDK.
 *
 * @param Options structure containing the account credentials to use during the link account operation
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the link account operation completes, either successfully or in error
 */
void EOSSDK_Auth::LinkAccount(const EOS_Auth_LinkAccountOptions* Options, void* ClientData, const EOS_Auth_OnLinkAccountCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Auth_LinkAccountCallbackInfo& laci = res->CreateCallback<EOS_Auth_LinkAccountCallbackInfo>((CallbackFunc)CompletionDelegate);
    laci.ClientData = ClientData;
    laci.LocalUserId = Settings::Inst().userid;
    laci.PinGrantInfo = nullptr;

    if (Options == nullptr)
    {
        laci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        laci.ResultCode = EOS_EResult::EOS_UnexpectedError;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Deletes a previously received and locally stored persistent auth access token for the currently logged in user of the local device.
 * The access token is deleted in they keychain of the local user and a backend request is also made to revoke the token on the authentication server.
 *
 * @param Options structure containing operation input parameters
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the deletion operation completes, either successfully or in error
 */
void EOSSDK_Auth::DeletePersistentAuth(const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Auth_DeletePersistentAuthCallbackInfo& dpaci = res->CreateCallback<EOS_Auth_DeletePersistentAuthCallbackInfo>((CallbackFunc)CompletionDelegate);

    dpaci.ClientData = ClientData;

    if (Options == nullptr || Options->RefreshToken == nullptr)
    {
        dpaci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        dpaci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Contact the backend service to verify validity of an existing user auth token.
 * This function is intended for server-side use only.
 *
 * @param Options structure containing information about the auth token being verified
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the logout operation completes, either successfully or in error
 */
void EOSSDK_Auth::VerifyUserAuth(const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Auth_VerifyUserAuthCallbackInfo& vuaci = res->CreateCallback<EOS_Auth_VerifyUserAuthCallbackInfo>((CallbackFunc)CompletionDelegate);

    vuaci.ClientData = ClientData;
    if (Options == nullptr || Options->AuthToken == nullptr)
    {
        vuaci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        vuaci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of accounts that are logged in.
 *
 * @return the number of accounts logged in.
 */
int32_t EOSSDK_Auth::GetLoggedInAccountsCount()
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return (_logged_in ? 1 : 0);
}

/**
 * Fetch an account id that is logged in.
 *
 * @param Index an index into the list of logged in accounts. If the index is out of bounds, the returned account id will be invalid.
 *
 * @return the account id associated with the index passed
 */
EOS_EpicAccountId EOSSDK_Auth::GetLoggedInAccountByIndex(int32_t Index)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Index == 0)
        return Settings::Inst().userid;

    return nullptr;
}
/**
 * Fetches the login status for an account id.
 *
 * @param LocalUserId the account id of the user being queried
 *
 * @return the enum value of a user's login status
 */

EOS_ELoginStatus EOSSDK_Auth::GetLoginStatus(EOS_EpicAccountId LocalUserId)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (LocalUserId == Settings::Inst().userid)
        return (_logged_in ? EOS_ELoginStatus::EOS_LS_LoggedIn : EOS_ELoginStatus::EOS_LS_NotLoggedIn);

    return EOS_ELoginStatus::EOS_LS_NotLoggedIn;
}

EOS_EResult EOSSDK_Auth::CopyUserAuthTokenOld(EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    TRACE_FUNC();
    EOS_Auth_CopyUserAuthTokenOptions options;
    options.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_001;

    return CopyUserAuthToken(&options, LocalUserId, OutUserAuthToken);
}

/**
 * Fetches a user auth token for an account id.
 *
 * @param Options structure containing the api version of CopyUserAuthToken to use
 * @param LocalUserId the account id of the user being queried
 * @param OutUserAuthToken the auth token for the given user, if it exists and is valid, use EOS_Auth_Token_Release when finished
 *
 * @see EOS_Auth_Token_Release
 *
 * @return EOS_Success if the information is available and passed out in OutUserAuthToken
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the auth token is not found or expired.
 *
 */
EOS_EResult EOSSDK_Auth::CopyUserAuthToken(const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options->ApiVersion > EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST)
        return EOS_EResult::EOS_VersionMismatch;

    if (OutUserAuthToken == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (LocalUserId == Settings::Inst().userid)
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
    
    APP_LOG(Log::LogLevel::DEBUG, "Accountid not found: %p %s", LocalUserId, (LocalUserId == nullptr ? "" : LocalUserId->to_string().c_str()));
    return EOS_EResult::EOS_NotFound;
}

EOS_NotificationId EOSSDK_Auth::AddNotifyLoginStatusChangedOld(void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    TRACE_FUNC();
    EOS_Auth_AddNotifyLoginStatusChangedOptions options;
    options.ApiVersion = EOS_AUTH_ADDNOTIFYLOGINSTATUSCHANGED_API_001;

    return AddNotifyLoginStatusChanged(&options, ClientData, Notification);
}

/**
 * Register to receive login status updates.
 * @note must call RemoveNotifyLoginStatusChanged to remove the notification
 *
 * @param Options structure containing the api version of AddNotifyLoginStatusChanged to use
 * @param ClientData arbitrary data that is passed back to you in the callback
 * @param Notification a callback that is fired when the login status for a user changes
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Auth::AddNotifyLoginStatusChanged(const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Notification == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);

    EOS_Auth_LoginStatusChangedCallbackInfo& lscci = res->CreateCallback<EOS_Auth_LoginStatusChangedCallbackInfo>((CallbackFunc)Notification);
    lscci.ClientData = ClientData;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving login status updates.
 *
 * @param InId handle representing the registered callback
 */
void EOSSDK_Auth::RemoveNotifyLoginStatusChanged(EOS_NotificationId InId)
{
    TRACE_FUNC();
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
    GLOBAL_LOCK();

    //switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case callback_type::k_iCallback:
        //{
        //    callback_type& callback = res->GetCallback<callback_type>();
        //    // Free resources
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

}