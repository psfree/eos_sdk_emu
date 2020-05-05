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

using namespace sdk;

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
EOS_DECLARE_FUNC(void) EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->Login(Options, ClientData, CompletionDelegate);
}

/**
 * Signs the player out of the online service.
 *
 * @param Options structure containing information about which account to log out.
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the logout operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_Logout(EOS_HAuth Handle, const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->Logout(Options, ClientData, CompletionDelegate);
}

/**
 * Deletes a previously received and locally stored persistent auth access token for the currently logged in user of the local device.
 * The access token is deleted in they keychain of the local user and a backend request is also made to revoke the token on the authentication server.
 *
 * @param Options structure containing operation input parameters
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the deletion operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_DeletePersistentAuth(EOS_HAuth Handle, const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->DeletePersistentAuth(Options, ClientData, CompletionDelegate);
}

/**
 * Contact the backend service to verify validity of an existing user auth token.
 * This function is intended for server-side use only.
 *
 * @param Options structure containing information about the auth token being verified
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the logout operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_VerifyUserAuth(EOS_HAuth Handle, const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->VerifyUserAuth(Options, ClientData, CompletionDelegate);
}

/**
 * Fetch the number of accounts that are logged in.
 *
 * @return the number of accounts logged in.
 */
EOS_DECLARE_FUNC(int32_t) EOS_Auth_GetLoggedInAccountsCount(EOS_HAuth Handle)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->GetLoggedInAccountsCount();
}

/**
 * Fetch an account id that is logged in.
 *
 * @param Index an index into the list of logged in accounts. If the index is out of bounds, the returned account id will be invalid.
 *
 * @return the account id associated with the index passed
 */
EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetLoggedInAccountByIndex(EOS_HAuth Handle, int32_t Index)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->GetLoggedInAccountByIndex(Index);
}

/**
 * Fetches the login status for an account id.
 *
 * @param LocalUserId the account id of the user being queried
 *
 * @return the enum value of a user's login status
 */
EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->GetLoginStatus(LocalUserId);
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthToken(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->CopyUserAuthToken(Options, LocalUserId, OutUserAuthToken);
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
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChanged(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->AddNotifyLoginStatusChanged(Options, ClientData, Notification);
}

/**
 * Unregister from receiving login status updates.
 *
 * @param InId handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Auth_RemoveNotifyLoginStatusChanged(EOS_HAuth Handle, EOS_NotificationId InId)
{
    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->RemoveNotifyLoginStatusChanged(InId);
}


EOS_DECLARE_FUNC(void) EOS_Auth_Token_Release(EOS_Auth_Token* AuthToken)
{
    LOG(Log::LogLevel::TRACE, "");

    delete[] AuthToken->ExpiresAt;
    delete[] AuthToken->RefreshExpiresAt;
    delete AuthToken;
}

