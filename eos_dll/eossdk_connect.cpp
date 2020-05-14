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
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_Connect::alive_heartbeat_rate) EOSSDK_Connect::alive_heartbeat_rate;
decltype(EOSSDK_Connect::alive_heartbeat)      EOSSDK_Connect::alive_heartbeat;
decltype(EOSSDK_Connect::user_infos_rate)      EOSSDK_Connect::user_infos_rate;

EOSSDK_Connect::EOSSDK_Connect()
{
    _myself.first = GetProductUserId(generate_account_id_from_name(EOSSDK_Client::Inst().product_name + Settings::Inst().userid->to_string()));
    _myself.second.connected = false;
    _myself.second.infos.set_userid(Settings::Inst().userid->to_string());
    _myself.second.infos.set_displayname(Settings::Inst().username);

    LOG(Log::LogLevel::DEBUG, "Userid: %s, Productid: %s", Settings::Inst().userid->to_string().c_str(), _myself.first->to_string().c_str());
    GetNetwork().set_default_channel(_myself.first->to_string(), 0);
    GetNetwork().advertise_peer_id(_myself.first->to_string());
    GetNetwork().advertise(true);

    GetCB_Manager().register_callbacks(this);
    GetCB_Manager().register_frame(this);
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kConnect);
}

EOSSDK_Connect::~EOSSDK_Connect()
{
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kConnect);
    GetCB_Manager().unregister_frame(this);
    GetCB_Manager().unregister_callbacks(this);

    GetCB_Manager().remove_all_notifications(this);
}

std::pair<EOS_ProductUserId const, user_state_t>* EOSSDK_Connect::get_user_by_userid(EOS_EpicAccountId userid)
{
    auto it = std::find_if(_users.begin(), _users.end(), [&userid]( std::pair<EOS_ProductUserId const, user_state_t> &user )
    {
        return user.second.infos.userid() == userid->to_string();
    });
    if (it == _users.end())
        return nullptr;

    return &(*it);
}

std::pair<EOS_ProductUserId const, user_state_t>* EOSSDK_Connect::get_user_by_productid(EOS_ProductUserId productid)
{
    auto it = _users.find(productid);
    if (it == _users.end())
        return nullptr;

    return &(*it);
}

std::pair<EOS_ProductUserId const, user_state_t>* EOSSDK_Connect::get_user_by_name(std::string const& username)
{
    auto it = std::find_if(_users.begin(), _users.end(), [&username](std::pair<EOS_ProductUserId const, user_state_t>& user)
    {
        return user.second.infos.displayname() == username;
    });
    if (it == _users.end())
        return nullptr;

    return &(*it);
}

void EOSSDK_Connect::add_session(EOS_ProductUserId session_id, std::string const& session_name)
{
    auto& sessions = *_myself.second.infos.mutable_sessions();
    sessions[session_name] = session_id->to_string();
}

void EOSSDK_Connect::remove_session(EOS_ProductUserId session_id, std::string const& session_name)
{
    auto& sessions = *_myself.second.infos.mutable_sessions();
    auto it = sessions.find(session_name);
    if (it != sessions.end())
    {
        sessions.erase(it);
    }
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
    lci.LocalUserId = product_id();
    lci.ResultCode = EOS_EResult::EOS_Success;
    
    res->done = true;
    GetCB_Manager().add_callback(this, res);

    _myself.second.connected = true;
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
    cuci.LocalUserId = product_id();
    cuci.ResultCode = EOS_EResult::EOS_Connect_UserAlreadyExists;

    res->done = true;
    GetCB_Manager().add_callback(this, res);

    _myself.second.connected = true;
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
    
    pFrameResult_t res(new FrameResult);
    EOS_Connect_QueryExternalAccountMappingsCallbackInfo& qeamci = res->CreateCallback<EOS_Connect_QueryExternalAccountMappingsCallbackInfo>((CallbackFunc)CompletionDelegate);
    qeamci.ClientData = ClientData;
    qeamci.LocalUserId = product_id();

    if (Options == nullptr || Options->ExternalAccountIds == nullptr)
    {
        qeamci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        switch (Options->AccountIdType)
        {
            case EOS_EExternalAccountType::EOS_EAT_EPIC:
            {
                qeamci.ResultCode = EOS_EResult::EOS_Success;
            }
            break;
            default:
            {
                qeamci.ResultCode = EOS_EResult::EOS_Connect_ExternalServiceUnavailable;
            }
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
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

    if (Options == nullptr || Options->TargetExternalUserId == nullptr || Options->AccountIdType != EOS_EExternalAccountType::EOS_EAT_EPIC)
        return GetInvalidProductUserId();

    for (auto const& user : _users)
    {
        if (user.second.infos.userid() == Options->TargetExternalUserId)
        {
            return user.first;
        }
    }

    return GetInvalidProductUserId();
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

    if(Options == nullptr || Options->TargetProductUserId == nullptr || InOutBufferLength == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (OutBuffer != nullptr)
        *OutBuffer = 0;

    if (Options->AccountIdType != EOS_EExternalAccountType::EOS_EAT_EPIC)
    {
        *InOutBufferLength = 1;
        return EOS_EResult::EOS_NotFound;
    }

    if (*InOutBufferLength < (Options->TargetProductUserId->to_string().length() + 1))
    {
        *InOutBufferLength = Options->TargetProductUserId->to_string().length() + 1;
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(OutBuffer, Options->TargetProductUserId->to_string().c_str(), Options->TargetProductUserId->to_string().length() + 1);

    return EOS_EResult::EOS_Success;
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
        return product_id();

    return GetInvalidProductUserId();
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

    if (LocalUserId == product_id())
        return (_myself.second.connected ? EOS_ELoginStatus::EOS_LS_LoggedIn : EOS_ELoginStatus::EOS_LS_NotLoggedIn);

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
    aeci.LocalUserId = product_id();

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
    lscci.LocalUserId = product_id();

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


/**
 * Fetch the number of linked external accounts for a product user id.
 *
 * @param Options The Options associated with retrieving the external account info count.
 *
 * @see EOS_Connect_CopyProductUserExternalAccountByIndex
 *
 * @return Number of external accounts or 0 otherwise
 */
uint32_t EOSSDK_Connect::GetProductUserExternalAccountCount(const EOS_Connect_GetProductUserExternalAccountCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return 0;
}

/**
 * Fetch information about an external account linked to a product user id.
 * On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
 *
 * @param Options Structure containing the target index.
 * @param OutExternalAccountInfo The external account info data for the user with given index.
 *
 * @see EOS_Connect_ExternalAccountInfo_Release
 *
 * @return An EOS_EResult that indicates the external account data was copied into the OutExternalAccountInfo
 *         EOS_Success if the information is available and passed out in OutExternalAccountInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the account data doesn't exist or hasn't been queried yet
 */
EOS_EResult EOSSDK_Connect::CopyProductUserExternalAccountByIndex(const EOS_Connect_CopyProductUserExternalAccountByIndexOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch information about an external account of a specific type linked to a product user id.
 * On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
 *
 * @param Options Structure containing the target external account type.
 * @param OutExternalAccountInfo The external account info data for the user with given external account type.
 *
 * @see EOS_Connect_ExternalAccountInfo_Release
 *
 * @return An EOS_EResult that indicates the external account data was copied into the OutExternalAccountInfo
 *         EOS_Success if the information is available and passed out in OutExternalAccountInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the account data doesn't exist or hasn't been queried yet
 */
EOS_EResult EOSSDK_Connect::CopyProductUserExternalAccountByAccountType(const EOS_Connect_CopyProductUserExternalAccountByAccountTypeOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch information about an external account linked to a product user id.
 * On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
 *
 * @param Options Structure containing the target external account id.
 * @param OutExternalAccountInfo The external account info data for the user with given external account id.
 *
 * @see EOS_Connect_ExternalAccountInfo_Release
 *
 * @return An EOS_EResult that indicates the external account data was copied into the OutExternalAccountInfo
 *         EOS_Success if the information is available and passed out in OutExternalAccountInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the account data doesn't exist or hasn't been queried yet
 */
EOS_EResult EOSSDK_Connect::CopyProductUserExternalAccountByAccountId(const EOS_Connect_CopyProductUserExternalAccountByAccountIdOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetch information about a Product User, using the external account that they most recently logged in with as the reference.
 * On a successful call, the caller must release the returned structure using the EOS_Connect_ExternalAccountInfo_Release API.
 *
 * @param Options Structure containing the target external account id.
 * @param OutExternalAccountInfo The external account info data last logged in for the user.
 *
 * @see EOS_Connect_ExternalAccountInfo_Release
 *
 * @return An EOS_EResult that indicates the external account data was copied into the OutExternalAccountInfo
 *         EOS_Success if the information is available and passed out in OutExternalAccountInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the account data doesn't exist or hasn't been queried yet
 */
EOS_EResult EOSSDK_Connect::CopyProductUserInfo(const EOS_Connect_CopyProductUserInfoOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Connect::send_connect_heartbeat(Connect_Heartbeat_pb* hb) const
{
    LOG(Log::LogLevel::TRACE, "");
    std::string const& user_id = product_id()->to_string();

    Network_Message_pb msg;
    Connect_Message_pb* conn = new Connect_Message_pb;

    conn->set_allocated_heartbeat(hb);

    msg.set_source_id(user_id);
    msg.set_allocated_connect(conn);

    GetNetwork().TCPSendToAllPeers(msg);
    return true;
}

bool EOSSDK_Connect::send_connect_infos_request(Network::peer_t const& peerid, Connect_Request_Info_pb* req) const
{
    LOG(Log::LogLevel::TRACE, "");
    std::string const& user_id = product_id()->to_string();

    Network_Message_pb msg;
    Connect_Message_pb* conn = new Connect_Message_pb;

    conn->set_allocated_request(req);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);
    msg.set_allocated_connect(conn);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Connect::send_connect_infos(Network::peer_t const& peerid, Connect_Infos_pb* infos) const
{
    LOG(Log::LogLevel::TRACE, "");
    std::string const& user_id = product_id()->to_string();

    Network_Message_pb msg;
    Connect_Message_pb* conn = new Connect_Message_pb;

    conn->set_allocated_infos(infos);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);
    msg.set_allocated_connect(conn);

    return GetNetwork().TCPSendTo(msg);
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Connect::on_connect_heartbeat(Network_Message_pb const& msg, Connect_Heartbeat_pb const& hb)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    auto &user = _users[GetProductUserId(msg.source_id())];

    user.connected = true;
    user.last_hearbeat = std::chrono::steady_clock::now();

    return true;
}

bool EOSSDK_Connect::on_connect_infos_request(Network_Message_pb const& msg, Connect_Request_Info_pb const& req)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    Connect_Infos_pb* infos = new Connect_Infos_pb;

    infos->set_userid(Settings::Inst().userid->to_string());

    return send_connect_infos(msg.source_id(), infos);
}

bool EOSSDK_Connect::on_connect_infos(Network_Message_pb const& msg, Connect_Infos_pb const& infos)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    auto& user = _users[GetProductUserId(msg.source_id())];
    auto now = std::chrono::steady_clock::now();

    user.infos = infos;
    user.last_infos = now;

    std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(&GetEOS_Friends(), EOS_Friends_OnFriendsUpdateInfo::k_iCallback));
    for (auto& notif : notifs)
    {
        EOS_Friends_OnFriendsUpdateInfo& ofui = notif->GetCallback<EOS_Friends_OnFriendsUpdateInfo>();
        ofui.TargetUserId = GetEpicUserId(user.infos.userid());
        notif->res.cb_func(notif->res.data);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Connect::CBRunFrame()
{
    GLOBAL_LOCK();

    if (!_myself.second.connected)
        return true;

    auto now = std::chrono::steady_clock::now();

    if ((now - _myself.second.last_hearbeat) > alive_heartbeat_rate)
    {
        Connect_Heartbeat_pb* hb = new Connect_Heartbeat_pb;
        send_connect_heartbeat(hb);
        _myself.second.last_hearbeat = now;
    }

    for (auto& user : _users)
    {
        if (!user.second.connected)
            continue;

        if ((now - user.second.last_hearbeat) > alive_heartbeat)
        {
            LOG(Log::LogLevel::DEBUG, "User disconnected (pid=%s, uid=%s)", user.first->to_string().c_str(), user.second.infos.userid().c_str());
            user.second.connected = false;
            GetEOS_Presence().set_user_status(GetEpicUserId(user.second.infos.userid()), EOS_Presence_EStatus::EOS_PS_Offline);
            continue;
        }

        if ((now - user.second.last_infos) > user_infos_rate)
        {
            Connect_Request_Info_pb* req = new Connect_Request_Info_pb;
            send_connect_infos_request(user.first->to_string(), req);
            user.second.last_infos = now;
        }
    }

    return true;
}

bool EOSSDK_Connect::RunNetwork(Network_Message_pb const& msg)
{
    if (GetProductUserId(msg.source_id()) == product_id())
        return true;

    Connect_Message_pb const& conn = msg.connect();

    switch (conn.message_case())
    {
        case Connect_Message_pb::MessageCase::kHeartbeat: return on_connect_heartbeat(msg, conn.heartbeat());
        case Connect_Message_pb::MessageCase::kRequest  : return on_connect_infos_request(msg, conn.request());
        case Connect_Message_pb::MessageCase::kInfos    : return on_connect_infos(msg, conn.infos());
        default: LOG(Log::LogLevel::WARN, "Unhandled network message %d", conn.message_case());
    }

    return true;
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
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case EOS_Connect_LoginCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_LoginCallbackInfo& callback = res->GetCallback<EOS_Connect_LoginCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_CreateUserCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_CreateUserCallbackInfo& callback = res->GetCallback<EOS_Connect_CreateUserCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_LinkAccountCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_LinkAccountCallbackInfo& callback = res->GetCallback<EOS_Connect_LinkAccountCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_CreateDeviceIdCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_CreateDeviceIdCallbackInfo& callback = res->GetCallback<EOS_Connect_CreateDeviceIdCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_DeleteDeviceIdCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_DeleteDeviceIdCallbackInfo& callback = res->GetCallback<EOS_Connect_DeleteDeviceIdCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_QueryExternalAccountMappingsCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_QueryExternalAccountMappingsCallbackInfo& callback = res->GetCallback<EOS_Connect_QueryExternalAccountMappingsCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_QueryProductUserIdMappingsCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_QueryProductUserIdMappingsCallbackInfo& callback = res->GetCallback<EOS_Connect_QueryProductUserIdMappingsCallbackInfo>();
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case EOS_Connect_AuthExpirationCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_AuthExpirationCallbackInfo& callback = res->GetCallback<EOS_Connect_AuthExpirationCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Connect_LoginStatusChangedCallbackInfo::k_iCallback:
        //{
        //    EOS_Connect_LoginStatusChangedCallbackInfo& callback = res->GetCallback<EOS_Connect_LoginStatusChangedCallbackInfo>();
        //}
        //break;
    }
}

}