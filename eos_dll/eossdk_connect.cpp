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

#include "eossdk_connect.h"
#include "eossdk_platform.h"
#include "settings.h"

namespace sdk
{

EOSSDK_Connect::EOSSDK_Connect():
    _productid(generate_account_id())
{
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_Connect::~EOSSDK_Connect()
{
    GetCB_Manager().unregister_callbacks(this);
    GetCB_Manager().remove_all_notifications(this);
}

/**
  * Login/Authenticate given a valid set of external auth credentials.
  *
  * @param Options structure containing the external account credentials and type to use during the login operation
  * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate a callback that is fired when the login operation completes, either successfully or in error
  */
void EOSSDK_Connect::Login(const EOS_Connect_LoginOptions* Options, void* ClientData, const EOS_Connect_OnLoginCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    pFrameResult_t res(new FrameResult);
    EOS_Connect_LoginCallbackInfo& lci = res->CreateCallback<EOS_Connect_LoginCallbackInfo>((CallbackFunc)CompletionDelegate);

    lci.ClientData = ClientData;
    lci.ContinuanceToken = nullptr;
    lci.LocalUserId = &_productid;
    lci.ResultCode = EOS_EResult::EOS_Success;
    
    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Create an account association with the Epic Online Service as a product user given their external auth credentials
 *
 * @param Options structure containing a continuance token from a "user not found" response during Login (always try login first)
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the create operation completes, either successfully or in error
 */
void EOSSDK_Connect::CreateUser(const EOS_Connect_CreateUserOptions* Options, void* ClientData, const EOS_Connect_OnCreateUserCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    pFrameResult_t res(new FrameResult);
    EOS_Connect_CreateUserCallbackInfo& cuci = res->CreateCallback<EOS_Connect_CreateUserCallbackInfo>((CallbackFunc)CompletionDelegate);

    cuci.ClientData = ClientData;
    cuci.LocalUserId = &_productid;
    cuci.ResultCode = EOS_EResult::EOS_Connect_UserAlreadyExists;

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Link a set of external auth credentials with an existing product user on the Epic Online Service
 *
 * @param Options structure containing a continuance token from a "user not found" response during Login (always try login first) and a currently logged in user not already associated with this external auth provider
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the link operation completes, either successfully or in error
 */
void EOSSDK_Connect::LinkAccount(const EOS_Connect_LinkAccountOptions* Options, void* ClientData, const EOS_Connect_OnLinkAccountCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

}

/**
 * Create a new unique pseudo-account that can be used to identify the current user profile on the local device.
 *
 * This function is intended to be used by mobile games and PC games that wish to allow
 * a new user to start playing without requiring to login to the game using any user identity.
 * In addition to this, the Device ID feature is used to automatically login the local user
 * also when they have linked at least one external user account(s) with the local Device ID.
 *
 * It is possible to link many devices with the same user's account keyring using the Device ID feature.
 *
 * Linking a device later or immediately with a real user account will ensure that the player
 * will not lose their progress if they switch devices or lose the device at some point,
 * as they will be always able to login with one of their linked real accounts and also link
 * another new device with the user account associations keychain. Otherwise, without having
 * at least one permanent user account linked to the Device ID, the player would lose all of their
 * game data and progression permanently should something happen to their device or the local
 * user profile on the device.
 *
 * After a successful one-time CreateDeviceId operation, the game can login the local user
 * automatically on subsequent game starts with EOS_Connect_Login using the EOS_ECT_DEVICEID_ACCESS_TOKEN
 * credentials type. If a Device ID already exists for the local user on the device then EOS_DuplicateNotAllowed
 * error result is returned and the caller should proceed to calling EOS_Connect_Login directly.
 *
 * @param Options structure containing operation input parameters
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the create operation completes, either successfully or in error
 */
void EOSSDK_Connect::CreateDeviceId(const EOS_Connect_CreateDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnCreateDeviceIdCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

}

/**
 * Delete any existing Device ID access credentials for the current user profile on the local device.
 *
 * The deletion is permanent and it is not possible to recover lost game data and progression
 * if the Device ID had not been linked with at least one real external user account.
 *
 * @param Options structure containing operation input parameters
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the delete operation completes, either successfully or in error
 */
void EOSSDK_Connect::DeleteDeviceId(const EOS_Connect_DeleteDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnDeleteDeviceIdCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

}

/**
 * Retrieve the equivalent product user ids from a list of external account ids from supported account providers.  The values will be cached and retrievable via EOS_Connect_GetExternalAccountMapping
 *
 * @param Options structure containing a list of external account ids, in string form, to query for the product user id representation
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the query operation completes, either successfully or in error
 */
void EOSSDK_Connect::QueryExternalAccountMappings(const EOS_Connect_QueryExternalAccountMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryExternalAccountMappingsCallback
    CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

}

/**
 * Retrieve the equivalent account id mappings from a list of product user ids.  The values will be cached and retrievable via EOS_Connect_GetProductUserIdMapping
 *
 * @param Options structure containing a list of product user ids to query for the external account representation
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the query operation completes, either successfully or in error
 */
void EOSSDK_Connect::QueryProductUserIdMappings(const EOS_Connect_QueryProductUserIdMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryProductUserIdMappingsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

}

/**
 * Fetch a product user id that maps to an external account id
 *
 * @param Options structure containing the local user and target external account id
 *
 * @return the product user id, previously retrieved from the backend service, for the given target external account
 */
EOS_ProductUserId EOSSDK_Connect::GetExternalAccountMapping(const EOS_Connect_GetExternalAccountMappingsOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    return nullptr;
}

/**
 * Fetch an external account id, in string form, that maps to a given product user id
 *
 * @param Options structure containing the local user and target product user id
 * @param OutBuffer The buffer into which the account id data should be written.  The buffer must be long enough to hold a string of EOS_CONNECT_EXTERNAL_ACCOUNT_ID_MAX_LENGTH.
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates the external account id was copied into the OutBuffer
 *         EOS_Success if the information is available and passed out in OutUserInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the mapping doesn't exist or hasn't been queried yet
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the external account id. InOutBufferLength contains the required minimum length to perform the operation successfully.
 */
EOS_EResult EOSSDK_Connect::GetProductUserIdMapping(const EOS_Connect_GetProductUserIdMappingOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    LOG(Log::LogLevel::TRACE, "");

    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch the number of product users that are logged in.
 *
 * @return the number of product users logged in.
 */
int32_t EOSSDK_Connect::GetLoggedInUsersCount()
{
    LOG(Log::LogLevel::TRACE, "");

    return 1;
}

/**
 * Fetch a product user id that is logged in. This product user id is in the Epic Online Services namespace
 *
 * @param Index an index into the list of logged in users. If the index is out of bounds, the returned product user id will be invalid.
 *
 * @return the product user id associated with the index passed
 */
EOS_ProductUserId EOSSDK_Connect::GetLoggedInUserByIndex(int32_t Index)
{
    LOG(Log::LogLevel::TRACE, "");

    if (Index == 0)
        return &_productid;

    return nullptr;
}

/**
 * Fetches the login status for an product user id.  This product user id is considered logged in as long as the underlying access token has not expired.
 *
 * @param LocalUserId the product user id of the user being queried
 *
 * @return the enum value of a user's login status
 */
EOS_ELoginStatus EOSSDK_Connect::GetLoginStatus(EOS_ProductUserId LocalUserId)
{
    LOG(Log::LogLevel::TRACE, "");

    if (*LocalUserId == _productid)
        return EOS_ELoginStatus::EOS_LS_LoggedIn;

    return EOS_ELoginStatus::EOS_LS_NotLoggedIn;
}

/**
 * Register to receive upcoming authentication expiration notifications.
 * Notification is approximately 10 minutes prior to expiration.
 * Call EOS_Connect_Login again with valid third party credentials to refresh access
 *
 * @note must call RemoveNotifyAuthExpiration to remove the notification
 *
 * @param Options structure containing the API version of the callback to use
 * @param ClientData arbitrary data that is passed back to you in the callback
 * @param Notification a callback that is fired when the authentication is about to expire
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Connect::AddNotifyAuthExpiration(const EOS_Connect_AddNotifyAuthExpirationOptions* Options, void* ClientData, const EOS_Connect_OnAuthExpirationCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);

    EOS_Connect_AuthExpirationCallbackInfo& aeci = res->CreateCallback<EOS_Connect_AuthExpirationCallbackInfo>((CallbackFunc)Notification);

    aeci.ClientData = ClientData;
    aeci.LocalUserId = nullptr; // TODO!

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving expiration notifications.
 *
 * @param InId handle representing the registered callback
 */
void EOSSDK_Connect::RemoveNotifyAuthExpiration(EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * Register to receive user login status updates.
 * @note must call RemoveNotifyLoginStatusChanged to remove the notification
 *
 * @param Options structure containing the API version of the callback to use
 * @param ClientData arbitrary data that is passed back to you in the callback
 * @param Notification a callback that is fired when the login status for a user changes
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Connect::AddNotifyLoginStatusChanged(const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Connect_OnLoginStatusChangedCallback Notification)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);

    EOS_Connect_LoginStatusChangedCallbackInfo& lscci = res->CreateCallback<EOS_Connect_LoginStatusChangedCallbackInfo>((CallbackFunc)Notification);

    lscci.ClientData = ClientData;
    lscci.PreviousStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;
    lscci.CurrentStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;
    lscci.LocalUserId = nullptr; // TODO!

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving user login status updates.
 *
 * @param InId handle representing the registered callback
 */
void EOSSDK_Connect::RemoveNotifyLoginStatusChanged(EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    GetCB_Manager().remove_notification(this, InId);
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Connect::CBRunFrame()
{
    return false;
}

bool EOSSDK_Connect::RunNetwork(Network_Message_pb const& msg)
{
    return false;
}

bool EOSSDK_Connect::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Connect::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {

    }
}

}