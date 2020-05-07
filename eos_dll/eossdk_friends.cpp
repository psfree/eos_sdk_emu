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

#include "eossdk_friends.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_Friends::alive_heartbeat_rate) EOSSDK_Friends::alive_heartbeat_rate;
decltype(EOSSDK_Friends::alive_heartbeat)      EOSSDK_Friends::alive_heartbeat;
decltype(EOSSDK_Friends::friend_infos_rate)    EOSSDK_Friends::friend_infos_rate;

EOSSDK_Friends::EOSSDK_Friends()
{
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kFriends);
    GetCB_Manager().register_frame(this);
    GetCB_Manager().register_callbacks(this);
}

EOSSDK_Friends::~EOSSDK_Friends()
{
    GetCB_Manager().unregister_callbacks(this);
    GetCB_Manager().unregister_frame(this);
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kFriends);

    GetCB_Manager().remove_all_notifications(this);
}

friend_infos_t* EOSSDK_Friends::get_friend(std::string const& userid)
{
    auto it = _friends.find(userid);
    if (it == _friends.end())
        return nullptr;

    return &it->second;
}

std::pair<std::string const, friend_infos_t>* EOSSDK_Friends::get_friend_by_name(std::string const& username)
{
    auto it = std::find_if(_friends.begin(), _friends.end(), [&username]( std::pair<std::string const&, friend_infos_t> const& frd)
    {
        return frd.second.infos.displayname() == username;
    });
    if (it == _friends.end())
        return nullptr;

    return &(*it);
}

/**
  * Starts an asynchronous task that reads the user's friends list from the backend service, caching it for future use.
  *
  * @param Options structure containing the account for which to retrieve the friends list
  * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
  */
void EOSSDK_Friends::QueryFriends(const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
    
    pFrameResult_t res(new FrameResult);

    EOS_Friends_QueryFriendsCallbackInfo& qfci = res->CreateCallback<EOS_Friends_QueryFriendsCallbackInfo>((CallbackFunc)CompletionDelegate);
    qfci.ClientData = ClientData;
    qfci.LocalUserId = Settings::Inst().userid;
    qfci.ResultCode = EOS_EResult::EOS_Success;

    _friends_cache_for_query.clear();
    std::copy_if(_friends.begin(), _friends.end(), std::inserter(_friends_cache_for_query, _friends_cache_for_query.end()), []( std::pair<std::string const, friend_infos_t>& finfos )
    {
        return finfos.second.online;
    });

    res->done = true;

    GetCB_Manager().add_callback(this, res);
}

/**
 * Starts an asynchronous task that sends a friend invitation to another user. The completion delegate is executed after the backend response has been received.
 * It does not indicate that the target user has responded to the friend invitation.
 *
 * @param Options structure containing the account to send the invite from and the account to send the invite to
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOSSDK_Friends::SendInvite(const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
    

}

/**
 * Starts an asynchronous task that accepts a friend invitation from another user. The completion delegate is executed after the backend response has been received.
 *
 * @param Options structure containing the logged in account and the inviting account
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOSSDK_Friends::AcceptInvite(const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
    

}

/**
 * Starts an asynchronous task that rejects a friend invitation from another user. The completion delegate is executed after the backend response has been received.
 *
 * @param Options structure containing the logged in account and the inviting account
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
void EOSSDK_Friends::RejectInvite(const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
    

}

/**
 * Retrieves the number of friends on the friends list that has already been retrieved by the EOS_Friends_QueryFriends API.
 *
 * @param Options structure containing the account id of the friends list
 * @return the number of friends on the list
 *
 * @see EOS_Friends_GetFriendAtIndex
 */
int32_t EOSSDK_Friends::GetFriendsCount(const EOS_Friends_GetFriendsCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return static_cast<int32_t>(_friends_cache_for_query.size());
}

/**
 * Retrieves the account id of an entry from the friends list that has already been retrieved by the EOS_Friends_QueryFriends API.
 * The account id returned by this function may belong to an account that has been invited to be a friend or that has invited the local user to be a friend.
 * To determine if the account id returned by this function is a friend or a pending friend invitation, use the EOS_Friends_GetStatus function.
 *
 * @param Options structure containing the account id of the friends list and the index into the list
 * @return the account id of the friend. Note that if the index provided is out of bounds, the returned account id will be a "null" account id.
 *
 * @see EOS_Friends_GetFriendsCount
 * @see EOS_Friends_GetStatus
 */
EOS_EpicAccountId EOSSDK_Friends::GetFriendAtIndex(const EOS_Friends_GetFriendAtIndexOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (Options == nullptr || Options->Index >= _friends_cache_for_query.size())
        return nullptr;
    
    auto it = _friends_cache_for_query.begin();
    std::advance(it, Options->Index);

    return GetEpicUserId(it->first);
}

/**
 * Retrieve the friendship status between the local user and another user.
 *
 * @param Options structure containing the account id of the friend list to check and the account of the user to test friendship status
 * @return A value indicating whether the two accounts have a friendship, pending invites in either direction, or no relationship
 *         EOS_FS_Friends is returned for two users that have confirmed friendship
 *         EOS_FS_InviteSent is returned when the local user has sent a friend invitation but the other user has not accepted or rejected it
 *         EOS_FS_InviteReceived is returned when the other user has sent a friend invitation to the local user
 *         EOS_FS_NotFriends is returned when there is no known relationship
 *
 * @see EOS_EFriendsStatus
 */
EOS_EFriendsStatus EOSSDK_Friends::GetStatus(const EOS_Friends_GetStatusOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (Options == nullptr || Options->TargetUserId == nullptr)
        return EOS_EFriendsStatus::EOS_FS_NotFriends;

    friend_infos_t* finfos = get_friend(Options->TargetUserId->to_string());
    if(finfos == nullptr)
        return EOS_EFriendsStatus::EOS_FS_NotFriends;
    
    return static_cast<EOS_EFriendsStatus>(finfos->infos.status());
}

/**
 * Listen for changes to friends for a particular account.
 *
 * @param Options Information about who would like notifications.
 * @param ClientData This value is returned to the caller when FriendsUpdateHandler is invoked.
 * @param FriendsUpdateHandler The callback to be invoked when a change to any friend status changes.
 * @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
 */
EOS_NotificationId EOSSDK_Friends::AddNotifyFriendsUpdate(const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);
    
    EOS_Friends_OnFriendsUpdateInfo& ofup = res->CreateCallback<EOS_Friends_OnFriendsUpdateInfo>((CallbackFunc)FriendsUpdateHandler);
    ofup.ClientData = ClientData;
    ofup.LocalUserId = Settings::Inst().userid;
    ofup.TargetUserId = GetEpicUserId("null");
    ofup.PreviousStatus = EOS_EFriendsStatus::EOS_FS_Friends;
    ofup.CurrentStatus = EOS_EFriendsStatus::EOS_FS_Friends;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Stop listening for friends changes on a previously bound handler.
 *
 * @param NotificationId The previously bound notification ID.
 */
void EOSSDK_Friends::RemoveNotifyFriendsUpdate(EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    GetCB_Manager().remove_notification(this, NotificationId);
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Friends::send_heartbeat(Friend_Heartbeat_pb* hb)
{
    Network_Message_pb msg;

    Friends_Message_pb* frd = new Friends_Message_pb;

    frd->set_allocated_heartbeat(hb);
    msg.set_allocated_friends(frd);

    return GetNetwork().SendBroadcast(msg);
}

bool EOSSDK_Friends::send_friend_info_request(Network::peer_t const& peerid, Friend_Info_Request_pb* req)
{
    Network_Message_pb msg;

    Friends_Message_pb* frd = new Friends_Message_pb;

    frd->set_allocated_friend_info_request(req);
    msg.set_allocated_friends(frd);

    msg.set_dest_id(peerid);

    return GetNetwork().SendTo(msg);
}

bool EOSSDK_Friends::send_friend_info(Network::peer_t const& peerid, Friend_Info_pb* infos)
{
    Network_Message_pb msg;

    Friends_Message_pb* frd = new Friends_Message_pb;

    frd->set_allocated_friend_info(infos);
    msg.set_allocated_friends(frd);

    msg.set_dest_id(peerid);

    return GetNetwork().SendTo(msg);
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Friends::on_heartbeat(Network_Message_pb const& msg, Friend_Heartbeat_pb const& hb)
{
    GLOBAL_LOCK();

    auto& finfos = _friends[msg.source_id()];
    finfos.online = true;
    finfos.last_hearbeat = std::chrono::steady_clock::now();

    return true;
}

bool EOSSDK_Friends::on_friend_info_request(Network_Message_pb const& msg, Friend_Info_Request_pb const& req)
{
    GLOBAL_LOCK();

    Friend_Info_pb* infos = new Friend_Info_pb;

    infos->set_displayname(Settings::Inst().username);
    return send_friend_info(msg.source_id(), infos);
}

bool EOSSDK_Friends::on_friend_info(Network_Message_pb const& msg, Friend_Info_pb const& infos)
{
    GLOBAL_LOCK();

    auto& finfos = _friends[msg.source_id()];
    finfos.last_infos = std::chrono::steady_clock::now();
    finfos.infos = infos;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Friends::CBRunFrame()
{
    GLOBAL_LOCK();

    auto now = std::chrono::steady_clock::now();

    for (auto& frd : _friends)
    {
        if (!frd.second.online)
            continue;

        if ((now - frd.second.last_hearbeat) > alive_heartbeat)
        {
            frd.second.online = false;
            continue;
        }

        if ((now - frd.second.last_infos) > friend_infos_rate)
        {
            Friend_Info_Request_pb* req = new Friend_Info_Request_pb;
            send_friend_info_request(frd.first, req);
        }
    }

    return false;
}

bool EOSSDK_Friends::RunNetwork(Network_Message_pb const& msg)
{
    if (msg.source_id() == Settings::Inst().userid->to_string())
        return true;

    Friends_Message_pb const& frd = msg.friends();

    switch (frd.message_case())
    {
        case Friends_Message_pb::MessageCase::kHeartbeat        : return on_heartbeat(msg, frd.heartbeat());
        case Friends_Message_pb::MessageCase::kFriendInfoRequest: return on_friend_info_request(msg, frd.friend_info_request());
        case Friends_Message_pb::MessageCase::kFriendInfo       : return on_friend_info(msg, frd.friend_info());
    }

    return true;
}

bool EOSSDK_Friends::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Friends::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case EOS_Friends_QueryFriendsCallbackInfo::k_iCallback:
        //{
        //    EOS_Friends_QueryFriendsCallbackInfo& callback = res->GetCallback<EOS_Friends_QueryFriendsCallbackInfo>();
        //    // Free resources
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case EOS_Friends_OnFriendsUpdateInfo::k_iCallback:
        //{
        //    EOS_Friends_OnFriendsUpdateInfo& ofup = res->GetCallback<EOS_Friends_OnFriendsUpdateInfo>();
        //}
        //break;
    }
}

}