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

#include "eossdk_lobby.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"

namespace sdk
{

/**
 * The Lobby Interface is used to manage lobbies that provide a persistent connection between users and
 * notifications of data sharing/updates.  Lobbies may also be found by advertising and searching with the backend service.
 * All Lobby Interface calls take a handle of type EOS_HLobby as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetLobbyInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetLobbyInterface
 */

EOSSDK_Lobby::EOSSDK_Lobby()
{
    GetCB_Manager().register_frame(this);
    GetCB_Manager().register_callbacks(this);
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kLobby);
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kLobbiesSearch);
}

EOSSDK_Lobby::~EOSSDK_Lobby()
{
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kLobbiesSearch);
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kLobby);
    GetCB_Manager().unregister_callbacks(this);
    GetCB_Manager().unregister_frame(this);
}

lobby_state_t* EOSSDK_Lobby::get_lobby_by_id(std::string const& lobby_id)
{
    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end())
        return nullptr;

    return &(it->second);
}

template<typename T>
bool compare_attribute_values(T&& v1, EOS_EOnlineComparisonOp op, T&& v2)
{
    try
    {
        switch (op)
        {
        case EOS_EOnlineComparisonOp::EOS_CO_EQUAL: return v1 == v2;
        case EOS_EOnlineComparisonOp::EOS_CO_NOTEQUAL: return v1 != v2;
        case EOS_EOnlineComparisonOp::EOS_CO_GREATERTHAN: return v1 > v2;
        case EOS_EOnlineComparisonOp::EOS_CO_GREATERTHANOREQUAL: return v1 >= v2;
        case EOS_EOnlineComparisonOp::EOS_CO_LESSTHAN: return v1 < v2;
        case EOS_EOnlineComparisonOp::EOS_CO_LESSTHANOREQUAL: return v1 <= v2;
        }
    }
    catch (...)
    {
        return false;
    }

    // Default return true
    return true;
}

std::vector<lobby_state_t*> EOSSDK_Lobby::get_lobby_from_attributes(google::protobuf::Map<std::string, Lobby_Search_Parameter> const& parameters)
{
    std::vector<lobby_state_t*> res;
    for (auto& lobby : _lobbies)
    {
        bool found = true;
        for (auto& param : parameters)
        {
            // Well known parameters
            //if (param.first == "bucket")
            //{
            //    auto& comparison = *param.second.param().begin();
            //
            //    switch (comparison.second.value_case())
            //    {
            //    case Session_Attr_Value::ValueCase::kS:
            //    {
            //        std::string const& s_session = session.second.infos.bucket_id();
            //        std::string const& s_search = comparison.second.s();
            //        found = compare_attribute_values(s_session, static_cast<EOS_EOnlineComparisonOp>(comparison.first), s_search);
            //    }
            //    break;
            //    default: found = false;
            //    }
            //}
            //else// Standard parameters
            {
                auto it = lobby.second.infos.attributes().find(param.first);
                if (it == lobby.second.infos.attributes().end())
                {
                    found = false;
                }
                else
                {
                    for (auto& comparisons : param.second.param())
                    {
                        // comparisons.first// Comparison type
                        if (comparisons.second.value_case() != it->second.value().value_case())
                        {
                            found = false;
                            break;
                        }

                        switch (comparisons.second.value_case())
                        {
                            case Session_Attr_Value::ValueCase::kB:
                            {
                                bool b_session = it->second.value().b();
                                bool b_search = comparisons.second.b();
                                found = compare_attribute_values(b_session, static_cast<EOS_EOnlineComparisonOp>(comparisons.first), b_search);
                            }
                            break;
                            case Session_Attr_Value::ValueCase::kI:
                            {
                                int64_t i_session = it->second.value().i();
                                int64_t i_search = comparisons.second.i();
                                found = compare_attribute_values(i_session, static_cast<EOS_EOnlineComparisonOp>(comparisons.first), i_search);
                            }
                            break;
                            case Session_Attr_Value::ValueCase::kD:
                            {
                                double i_session = it->second.value().d();
                                double i_search = comparisons.second.d();
                                found = compare_attribute_values(i_session, static_cast<EOS_EOnlineComparisonOp>(comparisons.first), i_search);
                            }
                            break;
                            case Session_Attr_Value::ValueCase::kS:
                            {
                                std::string const& s_session = it->second.value().s();
                                std::string const& s_search = comparisons.second.s();
                                found = compare_attribute_values(s_session, static_cast<EOS_EOnlineComparisonOp>(comparisons.first), s_search);
                            }
                            break;
                        }
                    }
                }
            }
            if (found == false)
                break;
        }

        if (found)
        {
            res.emplace_back(&lobby.second);
        }
    }

    return res;
}

void EOSSDK_Lobby::add_member_to_lobby(std::string const& member, lobby_state_t* lobby)
{
    if (lobby != nullptr)
    {
        (*lobby->infos.mutable_members())[member];
    }
}

void EOSSDK_Lobby::remove_member_from_lobby(std::string const& member, lobby_state_t* lobby)
{
    if (lobby != nullptr)
    {
        auto& members = *lobby->infos.mutable_members();
        auto it = members.find(member);
        if (it != members.end())
        {
            members.erase(it);
        }
    }
}

void EOSSDK_Lobby::promote_member(std::string const& member, lobby_state_t* lobby)
{
    if (lobby != nullptr)
    {
        // TODO: Promote owner
    }
}

void EOSSDK_Lobby::kick_member_from_lobby(std::string const& member, lobby_state_t* lobby)
{
    if (lobby != nullptr)
    {
        // TODO: Send kick network message
    }
}

bool EOSSDK_Lobby::is_member_in_lobby(std::string const& member, lobby_state_t* lobby)
{
    if (lobby != nullptr)
    {
        auto& members = lobby->infos.members();
        return (members.find(member) != members.end());
    }

    return false;
}

bool EOSSDK_Lobby::i_am_owner(lobby_state_t* lobby)
{
    if (lobby != nullptr)
    {
        return (GetProductUserId(lobby->infos.owner_id()) == GetEOS_Connect().get_myself()->first);
    }

    return false;
}

 /**
  * Creates a lobby and adds the user to the lobby membership.  There is no data associated with the lobby at the start and can be added vis EOS_Lobby_UpdateLobbyModification
  *
  * @param Options Required fields for the creation of a lobby such as a user count and its starting advertised state
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate A callback that is fired when the create operation completes, either successfully or in error
  *
  * @return EOS_Success if the creation completes successfully
  *         EOS_InvalidParameters if any of the options are incorrect
  *         EOS_LimitExceeded if the number of allowed lobbies is exceeded
  */
void EOSSDK_Lobby::CreateLobby(const EOS_Lobby_CreateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnCreateLobbyCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_CreateLobbyCallbackInfo& clci = res->CreateCallback<EOS_Lobby_CreateLobbyCallbackInfo>((CallbackFunc)CompletionDelegate);

    clci.ClientData = ClientData;
    
    {
        char* str = new char[sdk::max_id_length];
        *str = '\0';
        clci.LobbyId = str;
    }

    // Can't set a MaxLobbyMembers to less than the current member count
    if (Options == nullptr || Options->MaxLobbyMembers < 1 || Options->MaxLobbyMembers > EOS_LOBBY_MAX_LOBBY_MEMBERS)
    {
        clci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else if (_lobbies.size() >= EOS_LOBBY_MAX_LOBBIES)
    {
        clci.ResultCode = EOS_EResult::EOS_LimitExceeded;
    }
    else
    {
        std::string lobby_id(std::move(generate_account_id()));

        strncpy(const_cast<char*>(clci.LobbyId), lobby_id.c_str(), max_id_length);
        const_cast<char*>(clci.LobbyId)[64] = 0;

        auto& infos = _lobbies[lobby_id];
        infos.infos.set_lobby_id(lobby_id);
        infos.infos.set_owner_id(GetEOS_Connect().get_myself()->first->to_string());
        infos.infos.set_max_lobby_member(Options->MaxLobbyMembers);
        infos.infos.set_permission_level(get_enum_value(Options->PermissionLevel));
        (*infos.infos.mutable_members())[GetEOS_Connect().get_myself()->first->to_string()];
        infos.state = lobby_state_t::created;

        clci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Destroy a lobby given a lobby id
 *
 * @param Options Structure containing information about the lobby to be destroyed
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the destroy operation completes, either successfully or in error
 *
 * @return EOS_Success if the destroy completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_AlreadyPending if the lobby is already marked for destroy
 *         EOS_NotFound if the lobby to be destroyed does not exist
 */
void EOSSDK_Lobby::DestroyLobby(const EOS_Lobby_DestroyLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnDestroyLobbyCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_DestroyLobbyCallbackInfo& dlci = res->CreateCallback<EOS_Lobby_DestroyLobbyCallbackInfo>((CallbackFunc)CompletionDelegate);

    dlci.ClientData = ClientData;
    
    {
        char* str;
        if (Options->LobbyId == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = strlen(Options->LobbyId) + 1;
            str = new char[len];
            strncpy(str, Options->LobbyId, len);
        }
        dlci.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyId == nullptr)
    {
        dlci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        lobby_state_t* pLobby = get_lobby_by_id(Options->LobbyId);

        //send_lobby_promote_member(remote_id, ...);
        dlci.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Join a lobby, creating a local instance under a given lobby id.  Backend will validate various conditions to make sure it is possible to join the lobby.
 *
 * @param Options Structure containing information about the lobby to be joined
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the join operation completes, either successfully or in error
 *
 * @return EOS_Success if the destroy completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_Lobby::JoinLobby(const EOS_Lobby_JoinLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_JoinLobbyCallbackInfo& jlci = res->CreateCallback<EOS_Lobby_JoinLobbyCallbackInfo>((CallbackFunc)CompletionDelegate);

    EOSSDK_LobbyDetails* pLobbyDetails = reinterpret_cast<EOSSDK_LobbyDetails*>(Options->LobbyDetailsHandle);

    jlci.ClientData = ClientData;
    
    {
        char* str;
        if (pLobbyDetails == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = pLobbyDetails->_infos.lobby_id().length() + 1;
            str = new char[len];
            strncpy(str, pLobbyDetails->_infos.lobby_id().c_str(), len);
        }
        jlci.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyDetailsHandle == nullptr)
    {
        jlci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else if (_lobbies.size() >= EOS_LOBBY_MAX_LOBBIES)
    {
        jlci.ResultCode = EOS_EResult::EOS_LimitExceeded;
    }
    else
    {


        //send_lobby_promote_member(remote_id, ...);
        jlci.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Leave a lobby given a lobby id
 *
 * @param Options Structure containing information about the lobby to be left
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the leave operation completes, either successfully or in error
 *
 * @return EOS_Success if the leave completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_AlreadyPending if the lobby is already marked for leave
 *         EOS_NotFound if a lobby to be left does not exist
 */
void EOSSDK_Lobby::LeaveLobby(const EOS_Lobby_LeaveLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_LeaveLobbyCallbackInfo& llci = res->CreateCallback<EOS_Lobby_LeaveLobbyCallbackInfo>((CallbackFunc)CompletionDelegate);

    llci.ClientData = ClientData;
    
    {
        char* str;
        if (Options->LobbyId == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = strlen(Options->LobbyId) + 1;
            str = new char[len];
            strncpy(str, Options->LobbyId, len);
        }
        llci.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyId == nullptr)
    {
        llci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {


        //send_lobby_promote_member(remote_id, ...);
        llci.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Creates a lobby modification handle (EOS_HLobbyModification). The lobby modification handle is used to modify an existing lobby and can be applied with EOS_Lobby_UpdateLobby.
 * The EOS_HLobbyModification must be released by calling EOS_LobbyModification_Release once it is no longer needed.
 *
 * @param Options Required fields such as lobby id
 * @param OutLobbyModificationHandle Pointer to a Lobby Modification Handle only set if successful
 * @return EOS_Success if we successfully created the Lobby Modification Handle pointed at in OutLobbyModificationHandle, or an error result if the input data was invalid
 *		   EOS_InvalidParameters if any of the options are incorrect
 *
 * @see EOS_LobbyModification_Release
 * @see EOS_Lobby_UpdateLobby
 * @see EOS_LobbyModification_*
 */
EOS_EResult EOSSDK_Lobby::UpdateLobbyModification(const EOS_Lobby_UpdateLobbyModificationOptions* Options, EOS_HLobbyModification* OutLobbyModificationHandle)
{
    TRACE_FUNC();

    if (Options == nullptr || OutLobbyModificationHandle == nullptr)
    {
        *OutLobbyModificationHandle = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto lobby_it = _lobbies.find(Options->LobbyId);
    if (lobby_it == _lobbies.end())
    {
        *OutLobbyModificationHandle = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOSSDK_LobbyModification* pLobbyModif = new EOSSDK_LobbyModification;
    pLobbyModif->_infos = lobby_it->second.infos;

    *OutLobbyModificationHandle = reinterpret_cast<EOS_HLobbyModification>(pLobbyModif);

    return EOS_EResult::EOS_Success;
}

/**
 * Update a lobby given a lobby modification handle created via EOS_Lobby_UpdateLobbyModification
 *
 * @param Options Structure containing information about the lobby to be updated
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the update operation completes, either successfully or in error
 *
 * @return EOS_Success if the update completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Lobby_NotOwner if the lobby modification contains modifications that are only allowed by the owner
 *         EOS_NotFound if the lobby to update does not exist
 */
void EOSSDK_Lobby::UpdateLobby(const EOS_Lobby_UpdateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnUpdateLobbyCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_UpdateLobbyCallbackInfo& ulci = res->CreateCallback<EOS_Lobby_UpdateLobbyCallbackInfo>((CallbackFunc)CompletionDelegate);

    EOSSDK_LobbyModification* pLobbyModif = reinterpret_cast<EOSSDK_LobbyModification*>(Options->LobbyModificationHandle);
    
    ulci.ClientData = ClientData;

    {
        char* str;
        if (pLobbyModif == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = pLobbyModif->_infos.lobby_id().length() + 1;
            str = new char[len];
            strncpy(str, pLobbyModif->_infos.lobby_id().c_str(), len);
        }
        ulci.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyModificationHandle == nullptr)
    {
        ulci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        lobby_state_t* pLobby = get_lobby_by_id(pLobbyModif->_infos.lobby_id());
        if(pLobby == nullptr)
        {
            ulci.ResultCode = EOS_EResult::EOS_NotFound;
        }
        else
        {
            if (i_am_owner(pLobby))
            {
                pLobby->infos = pLobbyModif->_infos;
                ulci.ResultCode = EOS_EResult::EOS_Success;
                send_lobby_update(pLobby);
            }
            else
            {
                ulci.ResultCode = EOS_EResult::EOS_Lobby_NotOwner;
            }
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Promote an existing member of the lobby to owner, allowing them to make lobby data modifications
 *
 * @param Options Structure containing information about the lobby and member to be promoted
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the promotion operation completes, either successfully or in error
 *
 * @return EOS_Success if the promote completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Lobby_NotOwner if the calling user is not the owner of the lobby
 *         EOS_NotFound if the lobby of interest does not exist
 */
void EOSSDK_Lobby::PromoteMember(const EOS_Lobby_PromoteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnPromoteMemberCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_PromoteMemberCallbackInfo& pmci = res->CreateCallback<EOS_Lobby_PromoteMemberCallbackInfo>((CallbackFunc)CompletionDelegate);

    pmci.ClientData = ClientData;
    {
        char* str;
        if (Options->LobbyId == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = strlen(Options->LobbyId) + 1;
            str = new char[len];
            strncpy(str, Options->LobbyId, len);
        }
        pmci.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyId == nullptr || Options->TargetUserId == nullptr)
    {
        pmci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        //send_lobby_promote_member(remote_id, ...);
        pmci.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Kick an existing member from the lobby
 *
 * @param Options Structure containing information about the lobby and member to be kicked
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the kick operation completes, either successfully or in error
 *
 * @return EOS_Success if the kick completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Lobby_NotOwner if the calling user is not the owner of the lobby
 *         EOS_NotFound if a lobby of interest does not exist
 */
void EOSSDK_Lobby::KickMember(const EOS_Lobby_KickMemberOptions* Options, void* ClientData, const EOS_Lobby_OnKickMemberCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_KickMemberCallbackInfo& kmci = res->CreateCallback<EOS_Lobby_KickMemberCallbackInfo>((CallbackFunc)CompletionDelegate);

    kmci.ClientData = ClientData;
    {
        char* str;
        if (Options->LobbyId == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = strlen(Options->LobbyId) + 1;
            str = new char[len];
            strncpy(str, Options->LobbyId, len);
        }
        kmci.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyId == nullptr || Options->TargetUserId == nullptr)
    {
        kmci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        //send_lobby_kick_member(remote_id, ...);
        kmci.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Register to receive notifications when a lobby owner updates the attributes associated with the lobby.
 * @note must call RemoveNotifyLobbyUpdateReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Lobby::AddNotifyLobbyUpdateReceived(const EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyUpdateReceivedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new  FrameResult);
    EOS_Lobby_LobbyUpdateReceivedCallbackInfo& lurci = res->CreateCallback<EOS_Lobby_LobbyUpdateReceivedCallbackInfo>((CallbackFunc)NotificationFn);

    lurci.ClientData = ClientData;
    {
        char* str = new char[max_id_length];
        *str = '\0';
        lurci.LobbyId = str;
    }

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when a lobby changes its data.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Lobby::RemoveNotifyLobbyUpdateReceived(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * Register to receive notifications when a lobby member updates the attributes associated with themselves inside the lobby.
 * @note must call RemoveNotifyLobbyMemberUpdateReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Lobby::AddNotifyLobbyMemberUpdateReceived(const EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberUpdateReceivedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new  FrameResult);
    EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo& lmurci = res->CreateCallback<EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo>((CallbackFunc)NotificationFn);

    lmurci.ClientData = ClientData;
    {
        char* str = new char[max_id_length];
        *str = '\0';
        lmurci.LobbyId = str;
    }
    lmurci.TargetUserId = GetInvalidProductUserId();

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when lobby members change their data.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Lobby::RemoveNotifyLobbyMemberUpdateReceived(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * Register to receive notifications about the changing status of lobby members.
 * @note must call RemoveNotifyLobbyMemberStatusReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Lobby::AddNotifyLobbyMemberStatusReceived(const EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberStatusReceivedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new  FrameResult);
    EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo& lmsrci = res->CreateCallback<EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo>((CallbackFunc)NotificationFn);

    lmsrci.ClientData = ClientData;
    {
        char* str = new char[max_id_length];
        *str = '\0';
        lmsrci.LobbyId = str;
    }
    lmsrci.CurrentStatus = EOS_ELobbyMemberStatus::EOS_LMS_CLOSED;
    lmsrci.TargetUserId = GetInvalidProductUserId();

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when lobby members status change.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Lobby::RemoveNotifyLobbyMemberStatusReceived(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * Send an invite to another user.  User must be a member of the lobby or else the call will fail
 *
 * @param Options Structure containing information about the lobby and user to invite
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the send invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the send invite completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the lobby to send the invite from does not exist
 */
void EOSSDK_Lobby::SendInvite(const EOS_Lobby_SendInviteOptions* Options, void* ClientData, const EOS_Lobby_OnSendInviteCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_SendInviteCallbackInfo& sici = res->CreateCallback<EOS_Lobby_SendInviteCallbackInfo>((CallbackFunc)CompletionDelegate);

    sici.ClientData = ClientData;

    {
        char* str;
        if (Options->LobbyId == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = strlen(Options->LobbyId) + 1;
            str = new char[len];
            strncpy(str, Options->LobbyId, len);
        }
        sici.LobbyId = str;
    }

    if (Options == nullptr || Options->LobbyId == nullptr || Options->TargetUserId == nullptr)
    {
        sici.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        //send_lobby_invite(remote_id, lobby_invite);
        sici.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Reject an invite from another user.
 *
 * @param Options Structure containing information about the invite to reject
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the reject invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the invite rejection completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite does not exist
 */
void EOSSDK_Lobby::RejectInvite(const EOS_Lobby_RejectInviteOptions* Options, void* ClientData, const EOS_Lobby_OnRejectInviteCallback CompletionDelegate)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_RejectInviteCallbackInfo& rici = res->CreateCallback<EOS_Lobby_RejectInviteCallbackInfo>((CallbackFunc)CompletionDelegate);

    rici.ClientData = ClientData;

    {
        char* str;
        if (Options->LobbyId == nullptr)
        {
            str = new char[1];
            *str = '\0';
        }
        else
        {
            size_t len = strlen(Options->LobbyId) + 1;
            str = new char[len];
            strncpy(str, Options->LobbyId, len);
        }
        rici.LobbyId = str;
    }
    
    if (Options == nullptr || Options->LobbyId == nullptr)
    {
        rici.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        rici.ResultCode = EOS_EResult::EOS_NotFound;
    }

    res->done;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Retrieve all existing invites for a single user
 *
 * @param Options Structure containing information about the invites to query
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the query invites operation completes, either successfully or in error
 *
 */
void EOSSDK_Lobby::QueryInvites(const EOS_Lobby_QueryInvitesOptions* Options, void* ClientData, const EOS_Lobby_OnQueryInvitesCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Lobby_QueryInvitesCallbackInfo& qici = res->CreateCallback<EOS_Lobby_QueryInvitesCallbackInfo>((CallbackFunc)CompletionDelegate);

    qici.ClientData = ClientData;
    qici.LocalUserId = GetEOS_Connect().get_myself()->first;

    if (Options == nullptr)
    {
        qici.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        qici.ResultCode = EOS_EResult::EOS_Success;
    }
    
    res->done;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Get the number of known invites for a given user
 *
 * @param Options the Options associated with retrieving the current invite count
 *
 * @return number of known invites for a given user or 0 if there is an error
 */
uint32_t EOSSDK_Lobby::GetInviteCount(const EOS_Lobby_GetInviteCountOptions* Options)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (Options == nullptr)
        return 0;

    return 0;
}

/**
 * Retrieve an invite id from a list of active invites for a given user
 *
 * @param Options Structure containing the input parameters
 *
 * @return EOS_Success if the input is valid and an invite id was returned
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite doesn't exist
 *
 * @see EOS_Lobby_GetInviteCount
 * @see EOS_Lobby_CopyLobbyDetailsHandleByInviteId
 */
EOS_EResult EOSSDK_Lobby::GetInviteIdByIndex(const EOS_Lobby_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (Options == nullptr || /* Options->Index >= _invites.size() || */ InOutBufferLength == nullptr )
        return EOS_EResult::EOS_InvalidParameters;

    *InOutBufferLength = 0;

    if (OutBuffer != nullptr)
    {
        //strncpy(OutBuffer, lobby_id, lobby_length);
    }
    
    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Create a lobby search handle.  This handle may be modified to include various search parameters.
 * Searching is possible in three methods, all mutually exclusive
 * - set the lobby id to find a specific lobby
 * - set the target user id to find a specific user
 * - set lobby parameters to find an array of lobbies that match the search criteria (not available yet)
 *
 * @param Options Structure containing required parameters such as the maximum number of search results
 * @param OutLobbySearchHandle The new search handle or null if there was an error creating the search handle
 *
 * @return EOS_Success if the search creation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_EResult EOSSDK_Lobby::CreateLobbySearch(const EOS_Lobby_CreateLobbySearchOptions* Options, EOS_HLobbySearch* OutLobbySearchHandle)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->MaxResults == 0 || OutLobbySearchHandle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    _lobbies_searchs.emplace_back();
    EOSSDK_LobbySearch& search = _lobbies_searchs.back();
    search._max_results = Options->MaxResults;

    *OutLobbySearchHandle = reinterpret_cast<EOS_HLobbySearch>(&search);

    return EOS_EResult::EOS_Success;
}

/**
 * Register to receive notifications about lobby invites sent to local users.
 * @note must call RemoveNotifyLobbyInviteReceived to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Lobby::AddNotifyLobbyInviteReceived(const EOS_Lobby_AddNotifyLobbyInviteReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteReceivedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);

    EOS_Lobby_LobbyInviteReceivedCallbackInfo& lirci = res->CreateCallback<EOS_Lobby_LobbyInviteReceivedCallbackInfo>((CallbackFunc)NotificationFn);
    lirci.ClientData = ClientData;
    lirci.LocalUserId = GetEOS_Connect().get_myself()->first;
    lirci.TargetUserId = GetInvalidProductUserId();
    {
        char *str = new char[EOS_LOBBY_INVITEID_MAX_LENGTH + 1];
        *str = '\0';
        lirci.InviteId = str;
    }

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when a user receives a lobby invitation.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Lobby::RemoveNotifyLobbyInviteReceived(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * EOS_Lobby_CopyLobbyDetailsHandleByInviteId is used to immediately retrieve a handle to the lobby information from after notification of an invite
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutLobbyDetailsHandle out parameter used to receive the lobby handle
 *
 * @return EOS_Success if the information is available and passed out in OutLobbyDetailsHandle
 *         EOS_InvalidParameters if you pass an invalid invite id or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound If the invite id cannot be found
 *
 * @see EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions
 * @see EOS_LobbyDetails_Release
 */
EOS_EResult EOSSDK_Lobby::CopyLobbyDetailsHandleByInviteId(const EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    TRACE_FUNC();
    LOG(Log::LogLevel::INFO, "TODO");

    if (Options == nullptr || Options->InviteId == nullptr || OutLobbyDetailsHandle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;


    
    return EOS_EResult::EOS_NotFound;
}

/**
 * Create a handle to an existing lobby.
 * If the call returns an EOS_Success result, the out parameter, OutLobbyDetailsHandle, must be passed to EOS_LobbyDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing information about the lobby to retrieve
 * @param OutLobbyDetailsHandle The new active lobby handle or null if there was an error
 *
 * @return EOS_Success if the lobby handle was created successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the lobby doesn't exist
 */
EOS_EResult EOSSDK_Lobby::CopyLobbyDetailsHandle(const EOS_Lobby_CopyLobbyDetailsHandleOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle)
{
    TRACE_FUNC();

    if (Options == nullptr || OutLobbyDetailsHandle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto it = _lobbies.find(Options->LobbyId);
    if (it == _lobbies.end())
    {
        *OutLobbyDetailsHandle = nullptr;
        return EOS_EResult::EOS_NotFound;
    }
    
    EOSSDK_LobbyDetails* pLobbyDetails = new EOSSDK_LobbyDetails;

    pLobbyDetails->_infos = it->second.infos;

    *OutLobbyDetailsHandle = reinterpret_cast<EOS_HLobbyDetails>(pLobbyDetails);

    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Lobby::send_to_all_members(Network_Message_pb& msg, lobby_state_t* lobby)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    assert(lobby != nullptr);

    msg.set_source_id(user_id);
    for (auto const& member : lobby->infos.members())
    {
        if (member.first != user_id)
        {
            msg.set_dest_id(member.first);
            GetNetwork().TCPSendTo(msg);
        }
    }
    return true;
}

bool EOSSDK_Lobby::send_lobby_update(lobby_state_t* pLobby)
{
    TRACE_FUNC();
    
    Network_Message_pb msg;
    Lobby_Message_pb* lobby = new Lobby_Message_pb;
    
    Lobby_Update_pb* update = new Lobby_Update_pb;
    update->set_lobby_id(pLobby->infos.lobby_id());
    update->set_max_lobby_member(pLobby->infos.max_lobby_member());
    update->set_permission_level(pLobby->infos.permission_level());
    *update->mutable_attributes() = pLobby->infos.attributes();
    *update->mutable_members() = pLobby->infos.members();

    lobby->set_allocated_lobby_update(update);
    msg.set_allocated_lobby(lobby);

    return send_to_all_members(msg, pLobby);
}

bool EOSSDK_Lobby::send_lobbies_search_response(Network::peer_t const& peerid, Lobbies_Search_response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Lobbies_Search_Message_pb* search = new Lobbies_Search_Message_pb;

    search->set_allocated_search_response(resp);
    msg.set_allocated_lobbies_search(search);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Lobby::on_lobby_update(Network_Message_pb const& msg, Lobby_Update_pb const& update)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    lobby_state_t* pLobby = get_lobby_by_id(update.lobby_id());

    if (pLobby != nullptr)
    {
        std::vector<pFrameResult_t> notifs(std::move(GetCB_Manager().get_notifications(this, EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback)));
        for (auto& notif : notifs)
        {
            EOS_Lobby_LobbyUpdateReceivedCallbackInfo& lurci = notif->GetCallback<EOS_Lobby_LobbyUpdateReceivedCallbackInfo>();
            strncpy(const_cast<char*>(lurci.LobbyId), pLobby->infos.lobby_id().c_str(), max_id_length);
            notif->res.cb_func(notif->res.data);
        }
    }

    return true;
}

bool EOSSDK_Lobby::on_lobbies_search(Network_Message_pb const& msg, Lobbies_Search_pb const& search)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    Lobbies_Search_response_pb* resp = new Lobbies_Search_response_pb;
    resp->set_search_id(search.search_id());

    if (search.parameters_size() > 0)
    {
        std::vector<lobby_state_t*> lobbies = std::move(get_lobby_from_attributes(search.parameters()));
        for (auto& lobby : lobbies)
        {
            *resp->mutable_lobbies()->Add() = lobby->infos;
        }
    }
    else if(!search.lobby_id().empty())
    {
        lobby_state_t* pLobby = get_lobby_by_id(search.lobby_id());
        if (pLobby != nullptr)
        {
            *resp->mutable_lobbies()->Add() = pLobby->infos;
        }
    }
    else if (GetProductUserId(search.target_id()) == GetEOS_Connect().get_myself()->first)
    {
        for (auto& lobby : _lobbies)
        {
            *resp->mutable_lobbies()->Add() = lobby.second.infos;
        }
    }

    return send_lobbies_search_response(msg.source_id(), resp);
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Lobby::CBRunFrame()
{
    return false;
}

bool EOSSDK_Lobby::RunNetwork(Network_Message_pb const& msg)
{
    switch (msg.messages_case())
    {
        case Network_Message_pb::MessagesCase::kLobby:
        {
            Lobby_Message_pb const& lobby = msg.lobby();
            switch (lobby.message_case())
            {
                case Lobby_Message_pb::MessageCase::kLobbyUpdate      : return on_lobby_update(msg, lobby.lobby_update());
            }
        }
        break;

        case Network_Message_pb::MessagesCase::kLobbiesSearch:
        {
            Lobbies_Search_Message_pb const& search = msg.lobbies_search();
            switch (search.message_case())
            {
                case Lobbies_Search_Message_pb::MessageCase::kSearch: return on_lobbies_search(msg, search.search());
            }
        }
        break;
    }

    return true;
}

bool EOSSDK_Lobby::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;;
}

void EOSSDK_Lobby::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        case EOS_Lobby_CreateLobbyCallbackInfo::k_iCallback:
        {
            EOS_Lobby_CreateLobbyCallbackInfo& callback = res->GetCallback<EOS_Lobby_CreateLobbyCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_DestroyLobbyCallbackInfo::k_iCallback:
        {
            EOS_Lobby_DestroyLobbyCallbackInfo& callback = res->GetCallback<EOS_Lobby_DestroyLobbyCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_JoinLobbyCallbackInfo::k_iCallback:
        {
            EOS_Lobby_JoinLobbyCallbackInfo& callback = res->GetCallback<EOS_Lobby_JoinLobbyCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_LeaveLobbyCallbackInfo::k_iCallback:
        {
            EOS_Lobby_LeaveLobbyCallbackInfo& callback = res->GetCallback<EOS_Lobby_LeaveLobbyCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_UpdateLobbyCallbackInfo::k_iCallback:
        {
            EOS_Lobby_UpdateLobbyCallbackInfo& callback = res->GetCallback<EOS_Lobby_UpdateLobbyCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_PromoteMemberCallbackInfo::k_iCallback:
        {
            EOS_Lobby_PromoteMemberCallbackInfo& callback = res->GetCallback<EOS_Lobby_PromoteMemberCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_KickMemberCallbackInfo::k_iCallback:
        {
            EOS_Lobby_KickMemberCallbackInfo& callback = res->GetCallback<EOS_Lobby_KickMemberCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_SendInviteCallbackInfo::k_iCallback:
        {
            EOS_Lobby_SendInviteCallbackInfo& callback = res->GetCallback<EOS_Lobby_SendInviteCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_RejectInviteCallbackInfo::k_iCallback:
        {
            EOS_Lobby_RejectInviteCallbackInfo& callback = res->GetCallback<EOS_Lobby_RejectInviteCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        case EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback:
        {
            EOS_Lobby_LobbyUpdateReceivedCallbackInfo& callback = res->GetCallback<EOS_Lobby_LobbyUpdateReceivedCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo::k_iCallback:
        {
            EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo& callback = res->GetCallback<EOS_Lobby_LobbyMemberUpdateReceivedCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo::k_iCallback:
        {
            EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo& callback = res->GetCallback<EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo>();
            // Free resources
            delete[]callback.LobbyId;
        }
        break;

        case EOS_Lobby_LobbyInviteReceivedCallbackInfo::k_iCallback:
        {
            EOS_Lobby_LobbyInviteReceivedCallbackInfo& callback = res->GetCallback<EOS_Lobby_LobbyInviteReceivedCallbackInfo>();
            // Free resources
            delete[]callback.InviteId;
        }
        break;

    }
}

}