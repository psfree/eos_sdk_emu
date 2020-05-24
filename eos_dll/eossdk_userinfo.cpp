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

#include "eossdk_userinfo.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_UserInfo::userinfo_query_timeout) EOSSDK_UserInfo::userinfo_query_timeout;

EOSSDK_UserInfo::EOSSDK_UserInfo()
{
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kUserinfo);
    GetCB_Manager().register_callbacks(this);
    GetCB_Manager().register_frame(this);
}

EOSSDK_UserInfo::~EOSSDK_UserInfo()
{
    GetCB_Manager().unregister_frame(this);
    GetCB_Manager().unregister_callbacks(this);
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kUserinfo);
}

void EOSSDK_UserInfo::setup_myself()
{
    auto& userinfo = get_myself();

    userinfo.set_country("");
    userinfo.set_displayname(Settings::Inst().username);
    userinfo.set_nickname(Settings::Inst().username);
    userinfo.set_preferredlanguage(Settings::Inst().language);
}

UserInfo_Info_pb& EOSSDK_UserInfo::get_myself()
{
    return _userinfos[Settings::Inst().userid];
}

UserInfo_Info_pb* EOSSDK_UserInfo::get_userinfo(EOS_EpicAccountId userid)
{
    auto it = _userinfos.find(userid);
    if (it != _userinfos.end())
        return &it->second;

    return nullptr;
}

/**
  * EOS_UserInfo_QueryUserInfo is used to start an asynchronous query to retrieve information, such as display name, about another account.
  * Once the callback has been fired with a successful ResultCode, it is possible to call EOS_UserInfo_CopyUserInfo to receive an EOS_UserInfo containing the available information.
  *
  * @param Options structure containing the input parameters
  * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
  *
  * @see EOS_UserInfo
  * @see EOS_UserInfo_CopyUserInfo
  * @see EOS_UserInfo_QueryUserInfoOptions
  * @see EOS_UserInfo_OnQueryUserInfoCallback
  */
void EOSSDK_UserInfo::QueryUserInfo(const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_UserInfo_QueryUserInfoCallbackInfo& quici = res->CreateCallback<EOS_UserInfo_QueryUserInfoCallbackInfo>((CallbackFunc)CompletionDelegate);
    quici.ClientData = ClientData;
    quici.LocalUserId = Settings::Inst().userid;

    if (Options == nullptr || Options->TargetUserId == nullptr)
    {
        quici.TargetUserId = GetEpicUserId(sdk::NULL_USER_ID);
        quici.ResultCode = EOS_EResult::EOS_InvalidParameters;

        res->done = true;
    }
    else
    {
        quici.TargetUserId = Options->TargetUserId;

        auto *user = GetEOS_Connect().get_user_by_userid(Options->TargetUserId);
        if (user == nullptr)
        {
            quici.ResultCode = EOS_EResult::EOS_NotFound;
            res->done = true;
        }
        else if (user->first == GetEOS_Connect().product_id())
        {
            quici.ResultCode = EOS_EResult::EOS_Success;
            res->done = true;
        }
        else if(user->second.connected)
        {
            _userinfos_queries[Options->TargetUserId].push_back(res);

            UserInfo_Info_Request_pb* request = new UserInfo_Info_Request_pb;
            send_userinfo_request(user->first->to_string(), request);
        }
        else
        {
            quici.ResultCode = EOS_EResult::EOS_NotFound;
            res->done = true;
        }
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * EOS_UserInfo_QueryUserInfoByDisplayName is used to start an asynchronous query to retrieve user information by display name. This can be useful for getting the EOS_EpicAccountId for a display name.
 * Once the callback has been fired with a successful ResultCode, it is possible to call EOS_UserInfo_CopyUserInfo to receive an EOS_UserInfo containing the available information.
 *
 * @param Options structure containing the input parameters
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 *
 * @see EOS_UserInfo
 * @see EOS_UserInfo_CopyUserInfo
 * @see EOS_UserInfo_QueryUserInfoByDisplayNameOptions
 * @see EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback
 */
void EOSSDK_UserInfo::QueryUserInfoByDisplayName(const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo& quibdnci = res->CreateCallback<EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo>((CallbackFunc)CompletionDelegate);
    quibdnci.ClientData = ClientData;
    quibdnci.LocalUserId = Settings::Inst().userid;

    if (Options == nullptr || Options->DisplayName == nullptr)
    {
        quibdnci.TargetUserId = GetEpicUserId(sdk::NULL_USER_ID);
        quibdnci.ResultCode = EOS_EResult::EOS_InvalidParameters;

        res->done = true;
    }
    else
    {
        auto* user = GetEOS_Connect().get_user_by_name(Options->DisplayName);
        if (user == nullptr || !user->second.connected)
        {
            quibdnci.ResultCode = EOS_EResult::EOS_NotFound;
            res->done = true;
        }
        else
        {
            quibdnci.TargetUserId = GetEpicUserId(user->second.infos.userid());
            _userinfos_queries[quibdnci.TargetUserId].push_back(res);

            UserInfo_Info_Request_pb* request = new UserInfo_Info_Request_pb;
            send_userinfo_request(user->first->to_string(), request);
        }
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * EOS_UserInfo_CopyUserInfo is used to immediately retrieve a copy of user information for an account ID, cached by a previous call to EOS_UserInfo_QueryUserInfo.
 * If the call returns an EOS_Success result, the out parameter, OutUserInfo, must be passed to EOS_UserInfo_Release to release the memory associated with it.
 *
 * @param Options structure containing the input parameters
 * @param OutUserInfo out parameter used to receive the EOS_UserInfo structure.
 *
 * @return EOS_Success if the information is available and passed out in OutUserInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the user info is not locally cached. The information must have been previously cached by a call to EOS_UserInfo_QueryUserInfo
 *
 * @see EOS_UserInfo
 * @see EOS_UserInfo_CopyUserInfoOptions
 * @see EOS_UserInfo_Release
 */
EOS_EResult EOSSDK_UserInfo::CopyUserInfo(const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo** OutUserInfo)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (OutUserInfo == nullptr || Options == nullptr || Options->TargetUserId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    UserInfo_Info_pb* userinfo = get_userinfo(Options->TargetUserId);

    EOS_UserInfo* infos = new EOS_UserInfo();
    *OutUserInfo = infos;

    if (userinfo == nullptr)
    {
        memset(infos, 0, sizeof(*infos));
        return EOS_EResult::EOS_NotFound;
    }
   
    infos->ApiVersion        = EOS_USERINFO_COPYUSERINFO_API_LATEST;
    infos->Country           = (userinfo->country().empty()           ? nullptr : userinfo->country().c_str());
    infos->PreferredLanguage = (userinfo->preferredlanguage().empty() ? nullptr : userinfo->preferredlanguage().c_str());
    infos->DisplayName       = (userinfo->displayname().empty()       ? nullptr : userinfo->displayname().c_str());
    infos->Nickname          = (userinfo->nickname().empty()          ? nullptr : userinfo->nickname().c_str());
    infos->UserId            = Options->TargetUserId;

    return EOS_EResult::EOS_Success;
}

/**
 * Fetch the number of external user infos that are cached locally.
 *
 * @param Options The options associated with retrieving the external user info count
 *
 * @see EOS_UserInfo_CopyExternalUserInfoByIndex
 *
 * @return The number of external user infos, or 0 if there is an error
 */
uint32_t EOSSDK_UserInfo::GetExternalUserInfoCount(const EOS_UserInfo_GetExternalUserInfoCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return 0;
}

/**
 * Fetches an external user info from a given index.
 *
 * @param Options Structure containing the index being accessed
 * @param OutExternalUserInfo The external user info. If it exists and is valid, use EOS_UserInfo_ExternalUserInfo_Release when finished
 *
 * @see EOS_UserInfo_ExternalUserInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutExternalUserInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the external user info is not found
 */
EOS_EResult EOSSDK_UserInfo::CopyExternalUserInfoByIndex(const EOS_UserInfo_CopyExternalUserInfoByIndexOptions* Options, EOS_UserInfo_ExternalUserInfo** OutExternalUserInfo)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetches an external user info for a given external account type.
 *
 * @param Options Structure containing the account type being accessed
 * @param OutExternalUserInfo The external user info. If it exists and is valid, use EOS_UserInfo_ExternalUserInfo_Release when finished
 *
 * @see EOS_UserInfo_ExternalUserInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutExternalUserInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the external user info is not found
 */
EOS_EResult EOSSDK_UserInfo::CopyExternalUserInfoByAccountType(const EOS_UserInfo_CopyExternalUserInfoByAccountTypeOptions* Options, EOS_UserInfo_ExternalUserInfo** OutExternalUserInfo)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetches an external user info for a given external account id.
 *
 * @param Options Structure containing the account id being accessed
 * @param OutExternalUserInfo The external user info. If it exists and is valid, use EOS_UserInfo_ExternalUserInfo_Release when finished
 *
 * @see EOS_UserInfo_ExternalUserInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutExternalUserInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the external user info is not found
 */
EOS_EResult EOSSDK_UserInfo::CopyExternalUserInfoByAccountId(const EOS_UserInfo_CopyExternalUserInfoByAccountIdOptions* Options, EOS_UserInfo_ExternalUserInfo** OutExternalUserInfo)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_NotFound;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_UserInfo::send_userinfo_request(Network::peer_t const& peerid, UserInfo_Info_Request_pb* req)
{
    Network_Message_pb msg;
    UserInfo_Message_pb* userinfo = new UserInfo_Message_pb;

    std::string const& userid = GetEOS_Connect().product_id()->to_string();

    userinfo->set_allocated_userinfo_info_request(req);
    msg.set_allocated_userinfo(userinfo);

    msg.set_source_id(userid);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_UserInfo::send_my_userinfo(Network::peer_t const& peerid)
{
    Network_Message_pb msg;
    UserInfo_Message_pb* userinfo = new UserInfo_Message_pb;

    std::string const& userid = GetEOS_Connect().product_id()->to_string();

    userinfo->set_allocated_userinfo_info(&get_myself());
    msg.set_allocated_userinfo(userinfo);

    msg.set_source_id(userid);
    msg.set_dest_id(peerid);

    auto res = GetNetwork().TCPSendTo(msg);
    userinfo->release_userinfo_info();

    return res;
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_UserInfo::on_userinfo_request(Network_Message_pb const& msg, UserInfo_Info_Request_pb const& req)
{
    GLOBAL_LOCK();

    return send_my_userinfo(msg.source_id());
}

bool EOSSDK_UserInfo::on_userinfo(Network_Message_pb const& msg, UserInfo_Info_pb const& infos)
{
    GLOBAL_LOCK();

    _userinfos[GetEpicUserId(msg.source_id())] = infos;
    auto it = _userinfos_queries.find(GetEpicUserId(msg.source_id()));
    if (it != _userinfos_queries.end())
    {
        auto result_it = it->second.begin();
        switch ((*result_it)->res.m_iCallback)
        {
            case EOS_UserInfo_QueryUserInfoCallbackInfo::k_iCallback:
            {
                EOS_UserInfo_QueryUserInfoCallbackInfo& quici = (*result_it)->GetCallback<EOS_UserInfo_QueryUserInfoCallbackInfo>();
                quici.ResultCode = EOS_EResult::EOS_Success;
            }
            break;
            case EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo::k_iCallback:
            {
                EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo& quibdnci = (*result_it)->GetCallback<EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo>();
                quibdnci.ResultCode = EOS_EResult::EOS_Success;
            }
            break;
        }

        (*result_it)->done = true;

        it->second.erase(result_it);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_UserInfo::CBRunFrame()
{
    GLOBAL_LOCK();

    for (auto& queries : _userinfos_queries)
    {
        for (auto query_it = queries.second.begin(); query_it != queries.second.end();)
        {
            if ((std::chrono::steady_clock::now() - (*query_it)->created_time) > userinfo_query_timeout)
            {
                switch ((*query_it)->res.m_iCallback)
                {
                    case EOS_UserInfo_QueryUserInfoCallbackInfo::k_iCallback:
                    {
                        EOS_UserInfo_QueryUserInfoCallbackInfo& quici = (*query_it)->GetCallback<EOS_UserInfo_QueryUserInfoCallbackInfo>();
                        quici.ResultCode = EOS_EResult::EOS_TimedOut;
                    }
                    break;
                    case EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo::k_iCallback:
                    {
                        EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo& quibdnci = (*query_it)->GetCallback<EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo>();
                        quibdnci.ResultCode = EOS_EResult::EOS_TimedOut;
                    }
                    break;
                }

                (*query_it)->done = true;
                query_it = queries.second.erase(query_it);
            }
            else
                ++query_it;
        }
    }

    return true;
}

bool EOSSDK_UserInfo::RunNetwork(Network_Message_pb const& msg)
{
    if (msg.source_id() == Settings::Inst().userid->to_string())
        return true;

    UserInfo_Message_pb const& userinfo = msg.userinfo();

    switch (userinfo.message_case())
    {
        case UserInfo_Message_pb::MessageCase::kUserinfoInfoRequest: return on_userinfo_request(msg, userinfo.userinfo_info_request());
        case UserInfo_Message_pb::MessageCase::kUserinfoInfo       : return on_userinfo(msg, userinfo.userinfo_info());
    }

    return true;
}

bool EOSSDK_UserInfo::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_UserInfo::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    //switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case EOS_UserInfo_QueryUserInfoCallbackInfo::k_iCallback:
        //{
        //    EOS_UserInfo_QueryUserInfoCallbackInfo& quici = res->GetCallback<EOS_UserInfo_QueryUserInfoCallbackInfo>();
        //}
        //break;
        //case EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo::k_iCallback:
        //{
        //    EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo& quibdnci = res->GetCallback<EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo>();
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
    }
}

}