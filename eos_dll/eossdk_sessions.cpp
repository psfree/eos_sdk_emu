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

#include "eossdk_sessions.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_Sessions::join_timeout) EOSSDK_Sessions::join_timeout;

EOSSDK_Sessions::EOSSDK_Sessions()
{
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kSession);
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kSessionsSearch);

    GetCB_Manager().register_callbacks(this);

    GetCB_Manager().register_frame(this);
    
}

EOSSDK_Sessions::~EOSSDK_Sessions()
{
    GetCB_Manager().unregister_frame(this);

    GetCB_Manager().unregister_callbacks(this);

    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kSessionsSearch);
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kSession);

    GetCB_Manager().remove_all_notifications(this);
}

template<typename T>
bool compare_attribute_values(T&& v1, EOS_EOnlineComparisonOp op, T&& v2)
{
    try
    {
        switch (op)
        {
            case EOS_EOnlineComparisonOp::EOS_CO_EQUAL             : return v1 == v2;
            case EOS_EOnlineComparisonOp::EOS_CO_NOTEQUAL          : return v1 != v2;
            case EOS_EOnlineComparisonOp::EOS_CO_GREATERTHAN       : return v1 >  v2;
            case EOS_EOnlineComparisonOp::EOS_CO_GREATERTHANOREQUAL: return v1 >= v2;
            case EOS_EOnlineComparisonOp::EOS_CO_LESSTHAN          : return v1 <  v2;
            case EOS_EOnlineComparisonOp::EOS_CO_LESSTHANOREQUAL   : return v1 <= v2;
        }
    }
    catch (...)
    {
        return false;
    }

    // Default return true
    return true;
}

session_state_t* EOSSDK_Sessions::get_session_by_name(std::string const& session_name)
{
    auto it = _sessions.find(session_name);
    if (it == _sessions.end())
        return nullptr;

    return &it->second;
}

session_state_t* EOSSDK_Sessions::get_session_by_id(std::string const& session_id)
{
    auto it = std::find_if(_sessions.begin(), _sessions.end(), [&session_id]( std::pair<std::string const, session_state_t>& infos)
    {
        return session_id == infos.second.infos.sessionid();
    });
    if (it == _sessions.end())
        return nullptr;

    return &it->second;
}

session_state_t* EOSSDK_Sessions::get_session_from_attributes(google::protobuf::Map<std::string, Search_Parameter> const& parameters)
{
    session_state_t* res = nullptr;
    for (auto& session : _sessions)
    {
        bool found = true;
        for (auto& param : parameters)
        {
            // Well known parameters
            if (param.first == "bucket")
            {
                auto& comparison = *param.second.param().begin();

                switch (comparison.second.value_case())
                {
                    case Session_Attr_Value::ValueCase::kS:
                    {
                        std::string const& s_session = session.second.infos.bucketid();
                        std::string const& s_search  = comparison.second.s();
                        found = compare_attribute_values(s_session, static_cast<EOS_EOnlineComparisonOp>(comparison.first), s_search);
                    }
                    break;
                    default: found = false;
                }
            }
            else// Standard parameters
            {
                auto it = session.second.infos.attributes().find(param.first);
                if (it == session.second.infos.attributes().end())
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
            res = &session.second;
        }
    }

    return res;
}

void EOSSDK_Sessions::add_player_to_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        if(!is_player_in_session(player, session))
            *session->infos.add_players() = player;
    }
}

void EOSSDK_Sessions::register_player_to_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        if(!is_player_registered(player, session))
            *session->infos.add_registered_players() = player;
    }
}

void EOSSDK_Sessions::remove_player_from_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        auto it = std::find(session->infos.players().begin(), session->infos.players().end(), player);
        if (it != session->infos.players().end())
            session->infos.mutable_players()->erase(it);

        it = std::find(session->infos.registered_players().begin(), session->infos.registered_players().end(), player);
        if (it != session->infos.registered_players().end())
            session->infos.mutable_registered_players()->erase(it);
    }
}

void EOSSDK_Sessions::unregister_player_from_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        auto it = std::find(session->infos.registered_players().begin(), session->infos.registered_players().end(), player);
        if (it != session->infos.registered_players().end())
            session->infos.mutable_registered_players()->erase(it);
    }
}

bool EOSSDK_Sessions::is_player_in_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        auto it = std::find(session->infos.players().begin(), session->infos.players().end(), player);
        return it != session->infos.players().end();
    }
    return false;
}

bool EOSSDK_Sessions::is_player_registered(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        auto it = std::find(session->infos.registered_players().begin(), session->infos.registered_players().end(), player);
        return it != session->infos.registered_players().end();
    }
    return false;
}

/**
  * Creates a session modification handle (EOS_HSessionModification).  The session modification handle is used to build a new session and can be applied with EOS_Sessions_UpdateSession
  * The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it no longer needed.
  *
  * @param Options Required fields for the creation of a session such as a name, bucketid, and max players
  * @param OutSessionModificationHandle Pointer to a Session Modification Handle only set if successful
  * @return EOS_Success if we successfully created the Session Modification Handle pointed at in OutSessionModificationHandle, or an error result if the input data was invalid
  *
  * @see EOS_SessionModification_Release
  * @see EOS_Sessions_UpdateSession
  * @see EOS_SessionModification_*
  */
EOS_EResult EOSSDK_Sessions::CreateSessionModification(const EOS_Sessions_CreateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (OutSessionModificationHandle == nullptr || Options == nullptr || Options->SessionName == nullptr || Options->BucketId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    EOSSDK_SessionModification* modif = new EOSSDK_SessionModification;
    modif->_api_version = Options->ApiVersion;
    modif->_type = EOSSDK_SessionModification::modif_type::creation;

    switch (Options->ApiVersion)
    {
        case EOS_SESSIONS_CREATESESSIONMODIFICATION_API_002:
        {
            const EOS_Sessions_CreateSessionModificationOptions002* opts = reinterpret_cast<const EOS_Sessions_CreateSessionModificationOptions002*>(Options);
            modif->_infos.set_presence_allowed(opts->bPresenceEnabled);
        }

        case EOS_SESSIONS_CREATESESSIONMODIFICATION_API_001:
        {
            const EOS_Sessions_CreateSessionModificationOptions001* opts = reinterpret_cast<const EOS_Sessions_CreateSessionModificationOptions001*>(Options);
            modif->_infos.set_bucketid(opts->BucketId);
            modif->_infos.set_maxplayers(opts->MaxPlayers);
            modif->_infos.set_sessionname(opts->SessionName);
        }
    }
    

    *OutSessionModificationHandle = reinterpret_cast<EOS_HSessionModification>(modif);
    return EOS_EResult::EOS_Success;
}

/**
 * Creates a session modification handle (EOS_HSessionModification). The session modification handle is used to modify an existing session and can be applied with EOS_Sessions_UpdateSession.
 * The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it is no longer needed.
 *
 * @param Options Required fields such as session name
 * @param OutSessionModificationHandle Pointer to a Session Modification Handle only set if successful
 * @return EOS_Success if we successfully created the Session Modification Handle pointed at in OutSessionModificationHandle, or an error result if the input data was invalid
 *
 * @see EOS_SessionModification_Release
 * @see EOS_Sessions_UpdateSession
 * @see EOS_SessionModification_*
 */
EOS_EResult EOSSDK_Sessions::UpdateSessionModification(const EOS_Sessions_UpdateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (OutSessionModificationHandle == nullptr || Options == nullptr || Options->SessionName == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    EOSSDK_SessionModification* modif = new EOSSDK_SessionModification;
    modif->_api_version = Options->ApiVersion;
    modif->_type = EOSSDK_SessionModification::modif_type::update;

    auto session = get_session_by_name(Options->SessionName);
    if (session != nullptr)
    {
        modif->_infos = session->infos;
    }
    else
    {
        switch (Options->ApiVersion)
        {
            case EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_001:
            {
                const EOS_Sessions_UpdateSessionModificationOptions001* opts = reinterpret_cast<const EOS_Sessions_UpdateSessionModificationOptions001*>(Options);
                modif->_infos.set_sessionname(opts->SessionName);
            }
        }
    }
    
    *OutSessionModificationHandle = reinterpret_cast<EOS_HSessionModification>(modif);
    return EOS_EResult::EOS_Success;
}

/**
 * Update a session given a session modification handle created via EOS_Sessions_CreateSessionModification or EOS_Sessions_UpdateSessionModification
 *
 * @param Options Structure containing information about the session to be updated
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the update operation completes, either successfully or in error
 *
 * @return EOS_Success if the update completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be updated does not exist
 */
void EOSSDK_Sessions::UpdateSession(const EOS_Sessions_UpdateSessionOptions* Options, void* ClientData, const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_UpdateSessionCallbackInfo& usci = res->CreateCallback<EOS_Sessions_UpdateSessionCallbackInfo>((CallbackFunc)CompletionDelegate);
    usci.ClientData = ClientData;

    if (Options == nullptr || Options->SessionModificationHandle == nullptr)
    {
        usci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        {
            std::string const& sess_id = GetInvalidProductUserId()->to_string();
            char* session_id = new char[sess_id.length() + 1];
            strncpy(session_id, sess_id.c_str(), sess_id.length() + 1);
            usci.SessionId = session_id;
        }
        {
            char* str = new char[1];
            *str = 0;
            usci.SessionName = str;
        }
    }
    else
    {
        EOSSDK_SessionModification* modif = reinterpret_cast<EOSSDK_SessionModification*>(Options->SessionModificationHandle);
        usci.SessionId = nullptr;
        {
            std::string const& sess_name = modif->_infos.sessionname();
            char* name = new char[sess_name.length() + 1];
            strncpy(name, sess_name.c_str(), sess_name.length() + 1);
            usci.SessionName = name;
        }
        session_state_t* session = get_session_by_name(modif->_infos.sessionname());

        switch (modif->_type)
        {
            case EOSSDK_SessionModification::modif_type::creation:
            {
                if (session != nullptr)
                {
                    usci.ResultCode = EOS_EResult::EOS_Sessions_SessionAlreadyExists;
                }
                else
                {
                    auto& session = _sessions[modif->_infos.sessionname()];
                    {
                        modif->_infos.set_sessionid(generate_account_id());

                        std::string const& sess_id = modif->_infos.sessionid();
                        char* session_id = new char[sess_id.length() + 1];
                        strncpy(session_id, sess_id.c_str(), sess_id.length() + 1);
                        usci.SessionId = session_id;
                    }
                    session.state = session_state_t::state_e::created;
                    session.infos = modif->_infos;
                    session.infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Pending));
                    *session.infos.add_players() = GetEOS_Connect().product_id()->to_string();
                    *session.infos.add_registered_players() = GetEOS_Connect().product_id()->to_string();
                    GetEOS_Connect().add_session(GetProductUserId(session.infos.sessionid()), session.infos.sessionname());

                    usci.ResultCode = EOS_EResult::EOS_Success;
                }
            }
            break;

            case EOSSDK_SessionModification::modif_type::update  :
            {
                if (session == nullptr)
                {
                    usci.ResultCode = EOS_EResult::EOS_NotFound;
                }
                else
                {
                    modif->_infos.set_sessionid(session->infos.sessionid());
                    modif->_infos.set_state(session->infos.state());
                    session->infos = modif->_infos;
                    {
                        std::string const& sess_id = session->infos.sessionid();
                        char* session_id = new char[sess_id.length() + 1];
                        strncpy(session_id, sess_id.c_str(), sess_id.length() + 1);
                        usci.SessionId = session_id;
                    }
                    usci.ResultCode = EOS_EResult::EOS_Success;

                    send_session_info(session);
                }
            }
            break;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Destroy a session given a session name
 *
 * @param Options Structure containing information about the session to be destroyed
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the destroy operation completes, either successfully or in error
 *
 * @return EOS_Success if the destroy completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_AlreadyPending if the session is already marked for destroy
 *         EOS_NotFound if a session to be destroyed does not exist
 */
void EOSSDK_Sessions::DestroySession(const EOS_Sessions_DestroySessionOptions* Options, void* ClientData, const EOS_Sessions_OnDestroySessionCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_DestroySessionCallbackInfo& dsci = res->CreateCallback<EOS_Sessions_DestroySessionCallbackInfo>((CallbackFunc)CompletionDelegate);

    dsci.ClientData = ClientData;

    if (Options == nullptr || Options->SessionName == nullptr)
    {
        dsci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        auto it = _sessions.find(Options->SessionName);
        if (it != _sessions.end())
        {
            dsci.ResultCode = EOS_EResult::EOS_Success;
            if (it->second.state == session_state_t::state_e::created)
            {
                it->second.infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Destroying));

                Network_Message_pb msg;
                Session_Message_pb* session = new Session_Message_pb;
                Session_Destroy_pb* destroy = new Session_Destroy_pb;
                
                destroy->set_sessionid(it->second.infos.sessionid());
                destroy->set_sessionname(it->second.infos.sessionname());
                
                session->set_allocated_session_destroy(destroy);
                msg.set_allocated_session(session);

                send_to_all_members(msg, &it->second);
            }
            else
            {
                auto it = _sessions_join.find(Options->SessionName);
                if (it != _sessions_join.end())
                {
                    EOS_Sessions_JoinSessionCallbackInfo& jsci = it->second->GetCallback<EOS_Sessions_JoinSessionCallbackInfo>();
                    jsci.ResultCode = EOS_EResult::EOS_UnexpectedError;
                    res->done = true;
                    _sessions_join.erase(it);
                }
            }
            GetEOS_Connect().remove_session(GetProductUserId(it->second.infos.sessionid()), it->second.infos.sessionname());
            _sessions.erase(it);
        }
        else
        {
            dsci.ResultCode = EOS_EResult::EOS_NotFound;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Join a session, creating a local session under a given session name.  Backend will validate various conditions to make sure it is possible to join the session.
 *
 * @param Options Structure containing information about the session to be joined
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the join operation completes, either successfully or in error
 *
 * @return EOS_Success if the join completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_SessionAlreadyExists if the session is already exists or is in the process of being joined
 */
void EOSSDK_Sessions::JoinSession(const EOS_Sessions_JoinSessionOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Sessions_JoinSessionCallbackInfo& jsci = res->CreateCallback<EOS_Sessions_JoinSessionCallbackInfo>((CallbackFunc)CompletionDelegate);
    jsci.ClientData = ClientData;

    if (Options == nullptr || Options->SessionHandle == nullptr || Options->SessionName == nullptr)
    {
        jsci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        res->done = true;
    }
    else
    {
        if (_sessions.count(Options->SessionName) == 0) // If we haven't already a session with that name (created, joining or joined)
        {
            EOSSDK_SessionDetails* details = reinterpret_cast<EOSSDK_SessionDetails*>(Options->SessionHandle);

            Session_Join_Request_pb* join = new Session_Join_Request_pb;
            join->set_sessionid(details->_infos.sessionid());
            join->set_sessionname(Options->SessionName);

            session_state_t& session = _sessions[Options->SessionName];
            session.state = session_state_t::state_e::joining;
            session.infos = details->_infos;
            _sessions_join[Options->SessionName] = res;

            send_session_join_request(&session);
        }
        else
        {
            jsci.ResultCode = EOS_EResult::EOS_Sessions_SessionAlreadyExists;
            res->done = true;
        }
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * Mark a session as started, making it unable to find if session properties indicate "join in progress" is not available
 *
 * @param Options Structure containing information about the session to be started
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the start operation completes, either successfully or in error
 *
 * @return EOS_Success if the start completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be started does not exist
 */
void EOSSDK_Sessions::StartSession(const EOS_Sessions_StartSessionOptions* Options, void* ClientData, const EOS_Sessions_OnStartSessionCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Sessions_StartSessionCallbackInfo& ssci = res->CreateCallback<EOS_Sessions_StartSessionCallbackInfo>((CallbackFunc)CompletionDelegate);

    ssci.ClientData = ClientData;

    if (Options == nullptr || Options->SessionName == nullptr)
    {
        ssci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        auto session = get_session_by_name(Options->SessionName);
        if (session != nullptr)
        {
            ssci.ResultCode = EOS_EResult::EOS_Success;
            switch (session->infos.state())
            {
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Destroying):
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_NoSession) :
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Ending)    :
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Creating)  :
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Starting)  :
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_InProgress):
                    ssci.ResultCode = EOS_EResult::EOS_InvalidParameters;
                    break;

                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Ended)     :
                case get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Pending)   :
                    session->infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_InProgress));
                    send_session_info(session);
            }
        }
        else
        {
            ssci.ResultCode = EOS_EResult::EOS_NotFound;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Mark a session as ended, making it available to find if "join in progress" was disabled.  The session may be started again if desired
 *
 * @param Options Structure containing information about the session to be ended
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the end operation completes, either successfully or in error
 *
 * @return EOS_Success if the end completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be ended does not exist
 */
void EOSSDK_Sessions::EndSession(const EOS_Sessions_EndSessionOptions* Options, void* ClientData, const EOS_Sessions_OnEndSessionCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Sessions_EndSessionCallbackInfo& esci = res->CreateCallback<EOS_Sessions_EndSessionCallbackInfo>((CallbackFunc)CompletionDelegate);

    esci.ClientData = ClientData;

    if (Options == nullptr || Options->SessionName == nullptr)
    {
        esci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        auto session = get_session_by_name(Options->SessionName);
        if (session != nullptr)
        {
            esci.ResultCode = EOS_EResult::EOS_Success;
            session->infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Ended));
        }
        else
        {
            esci.ResultCode = EOS_EResult::EOS_NotFound;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Register a group of players with the session, allowing them to invite others or otherwise indicate they are part of the session for determining a full session
 *
 * @param Options Structure containing information about the session and players to be registered
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the registration operation completes, either successfully or in error
 *
 * @return EOS_Success if the register completes successfully
 *         EOS_NoChange if the players to register registered previously
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to register players does not exist
 */
void EOSSDK_Sessions::RegisterPlayers(const EOS_Sessions_RegisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_RegisterPlayersCallbackInfo& rpci = res->CreateCallback<EOS_Sessions_RegisterPlayersCallbackInfo>((CallbackFunc)CompletionDelegate);
    rpci.ClientData = ClientData;

    if (Options->SessionName == nullptr || Options->PlayersToRegister == nullptr || Options->PlayersToRegisterCount == 0)
    {
        rpci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        session_state_t* session = get_session_by_name(Options->SessionName);
        if (session == nullptr)
        {
            rpci.ResultCode = EOS_EResult::EOS_NotFound;
        }
        else
        {
            if (is_player_registered(GetEOS_Connect().product_id()->to_string(), session))
            {
                auto registered_players = session->infos.registered_players_size();
                for (uint32_t i = 0; i < Options->PlayersToRegisterCount; ++i)
                {
                    register_player_to_session(Options->PlayersToRegister[i]->to_string(), session);
                }
                if (registered_players == session->infos.registered_players_size())
                {
                    rpci.ResultCode = EOS_EResult::EOS_NoChange;
                }
                else
                {
                    rpci.ResultCode = EOS_EResult::EOS_Success;

                    Network_Message_pb msg;
                    Session_Message_pb* session_pb = new Session_Message_pb;

                    session_pb->set_allocated_session_info(&session->infos);
                    msg.set_allocated_session(session_pb);

                    send_to_all_members(msg, session);

                    session_pb->release_session_info();
                }
            }
            else
            {
                rpci.ResultCode = EOS_EResult::EOS_Sessions_NotAllowed;
            }
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Unregister a group of players with the session, freeing up space for others to join
 *
 * @param Options Structure containing information about the session and players to be unregistered
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the unregistration operation completes, either successfully or in error
 *
 * @return EOS_Success if the unregister completes successfully
 *         EOS_NoChange if the players to unregister were not found
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be unregister players does not exist
 */
void EOSSDK_Sessions::UnregisterPlayers(const EOS_Sessions_UnregisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_UnregisterPlayersCallbackInfo& upci = res->CreateCallback<EOS_Sessions_UnregisterPlayersCallbackInfo>((CallbackFunc)CompletionDelegate);
    upci.ClientData = ClientData;

    if (Options->SessionName == nullptr || Options->PlayersToUnregister == nullptr || Options->PlayersToUnregisterCount == 0)
    {
        upci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        session_state_t* session = get_session_by_name(Options->SessionName);
        if (session == nullptr)
        {
            upci.ResultCode = EOS_EResult::EOS_NotFound;
        }
        else
        {
            if (is_player_registered(GetEOS_Connect().product_id()->to_string(), session))
            {
                auto registered_players = session->infos.registered_players_size();
                for (uint32_t i = 0; i < Options->PlayersToUnregisterCount; ++i)
                {
                    unregister_player_from_session(Options->PlayersToUnregister[i]->to_string(), session);
                }
                if (registered_players == session->infos.registered_players_size())
                {
                    upci.ResultCode = EOS_EResult::EOS_NoChange;
                }
                else
                {
                    upci.ResultCode = EOS_EResult::EOS_Success;

                    Network_Message_pb msg;
                    Session_Message_pb* session_pb = new Session_Message_pb;

                    session_pb->set_allocated_session_info(&session->infos);
                    msg.set_allocated_session(session_pb);

                    send_to_all_members(msg, session);

                    session_pb->release_session_info();
                }
            }
            else
            {
                upci.ResultCode = EOS_EResult::EOS_Sessions_NotAllowed;
            }
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Send an invite to another player.  User must have created the session or be registered in the session or else the call will fail
 *
 * @param Options Structure containing information about the session and player to invite
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the send invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the send invite completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the session to send the invite from does not exist
 */
void EOSSDK_Sessions::SendInvite(const EOS_Sessions_SendInviteOptions* Options, void* ClientData, const EOS_Sessions_OnSendInviteCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_SendInviteCallbackInfo& sici = res->CreateCallback<EOS_Sessions_SendInviteCallbackInfo>((CallbackFunc)CompletionDelegate);

    sici.ClientData = ClientData;

    if (Options == nullptr || Options->SessionName == nullptr || Options->TargetUserId == nullptr)
    {
        sici.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        auto session = get_session_by_name(Options->SessionName);
        if (session == nullptr || GetEOS_Connect().get_user_by_productid(Options->TargetUserId) == nullptr)
        {
            sici.ResultCode = EOS_EResult::EOS_NotFound;
        }
        else
        {
            Session_Invite_pb* invite = new Session_Invite_pb;
            invite->set_sessionname(Options->SessionName);
            *invite->mutable_infos() = session->infos;
            send_session_invite(Options->TargetUserId->to_string(), invite);
            sici.ResultCode = EOS_EResult::EOS_Success;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Reject an invite from another player.
 *
 * @param Options Structure containing information about the invite to reject
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the reject invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the invite rejection completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite does not exist
 */
void EOSSDK_Sessions::RejectInvite(const EOS_Sessions_RejectInviteOptions* Options, void* ClientData, const EOS_Sessions_OnRejectInviteCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    
}

/**
 * Retrieve all existing invites for a single user
 *
 * @param Options Structure containing information about the invites to query
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the query invites operation completes, either successfully or in error
 *
 */
void EOSSDK_Sessions::QueryInvites(const EOS_Sessions_QueryInvitesOptions* Options, void* ClientData, const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_QueryInvitesCallbackInfo& qici = res->CreateCallback<EOS_Sessions_QueryInvitesCallbackInfo>((CallbackFunc)CompletionDelegate);
    qici.LocalUserId = GetEOS_Connect().get_myself()->first;
    qici.ClientData = ClientData;

    if (Options == nullptr || Options->LocalUserId == nullptr)
    {
        qici.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        qici.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Get the number of known invites for a given user
 *
 * @param Options the Options associated with retrieving the current invite count
 *
 * @return number of known invites for a given user or 0 if there is an error
 */
uint32_t EOSSDK_Sessions::GetInviteCount(const EOS_Sessions_GetInviteCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->LocalUserId != GetEOS_Connect().get_myself()->first)
        return 0;

    return _session_invites.size();
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
 * @see EOS_Sessions_GetInviteCount
 * @see EOS_Sessions_CopySessionHandleByInviteId
 */
EOS_EResult EOSSDK_Sessions::GetInviteIdByIndex(const EOS_Sessions_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->LocalUserId != GetEOS_Connect().get_myself()->first ||
        Options->Index >= _session_invites.size() ||
        OutBuffer == nullptr || InOutBufferLength == nullptr)
    {
        return EOS_EResult::EOS_InvalidParameters;
    }
    
    auto it = _session_invites.begin();
    std::advance(it, Options->Index);

    strncpy(OutBuffer, it->invite_id.c_str(), *InOutBufferLength);
    *InOutBufferLength = std::min<int32_t>(it->invite_id.length() + 1, *InOutBufferLength);

    return EOS_EResult::EOS_Success;
}

/**
 * Create a session search handle.  This handle may be modified to include various search parameters.
 * Searching is possible in three methods, all mutually exclusive
 * - set the session id to find a specific session
 * - set the target user id to find a specific user
 * - set session parameters to find an array of sessions that match the search criteria
 *
 * @param Options Structure containing required parameters such as the maximum number of search results
 * @param OutSessionSearchHandle The new search handle or null if there was an error creating the search handle
 *
 * @return EOS_Success if the search creation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_EResult EOSSDK_Sessions::CreateSessionSearch(const EOS_Sessions_CreateSessionSearchOptions* Options, EOS_HSessionSearch* OutSessionSearchHandle)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->MaxSearchResults == 0)
        return EOS_EResult::EOS_InvalidParameters;
    
    auto search_handle = new EOSSDK_SessionSearch();
    *OutSessionSearchHandle = reinterpret_cast<EOS_HSessionSearch>(search_handle);

    _session_searchs.emplace(search_handle);

    return EOS_EResult::EOS_Success;
}

/**
 * Create a handle to an existing active session.
 *
 * @param Options Structure containing information about the active session to retrieve
 * @param OutSessionHandle The new active session handle or null if there was an error
 *
 * @return EOS_Success if the session handle was created successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the active session doesn't exist
 */
EOS_EResult EOSSDK_Sessions::CopyActiveSessionHandle(const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options->SessionName == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    session_state_t* session = get_session_by_name(Options->SessionName);
    if (session == nullptr)
        return EOS_EResult::EOS_NotFound;

    EOSSDK_ActiveSession* active_session = new EOSSDK_ActiveSession;
    
    active_session->_infos = session->infos;

    *OutSessionHandle = reinterpret_cast<EOS_HActiveSession>(active_session);

    return EOS_EResult::EOS_Success;
}

/**
 * Register to receive session invites.
 * @note must call RemoveNotifySessionInviteReceived to remove the notification
 *
 * @param Options Structure containing information about the session invite notification
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param Notification A callback that is fired when a session invite for a user has been received
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Sessions::AddNotifySessionInviteReceived(const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn)
{
    TRACE_FUNC();
    
    if (NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);
    
    EOS_Sessions_SessionInviteReceivedCallbackInfo& sirci = res->CreateCallback<EOS_Sessions_SessionInviteReceivedCallbackInfo>((CallbackFunc)NotificationFn);

    sirci.ClientData = ClientData;
    sirci.LocalUserId = GetEOS_Connect().get_myself()->first;
    sirci.InviteId = new char[65];

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving session invites.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Sessions::RemoveNotifySessionInviteReceived(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * Register to receive notifications when a user accepts a session invite via the social overlay.
 * @note must call RemoveNotifySessionInviteAccepted to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Sessions::AddNotifySessionInviteAccepted(const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn)
{
    TRACE_FUNC();

    if (NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_SessionInviteAcceptedCallbackInfo& siaci = res->CreateCallback<EOS_Sessions_SessionInviteAcceptedCallbackInfo>((CallbackFunc)NotificationFn);

    siaci.ClientData = ClientData;
    siaci.LocalUserId = GetEOS_Connect().get_myself()->first;
    siaci.SessionId = new char[65];

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when a user accepts a session invite via the social overlay.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Sessions::RemoveNotifySessionInviteAccepted(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * EOS_Sessions_CopySessionHandleByInviteId is used to immediately retrieve a handle to the session information from after notification of an invite
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid invite id or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the invite id cannot be found
 *
 * @see EOS_Sessions_CopySessionHandleByInviteIdOptions
 * @see EOS_SessionDetails_Release
 */
EOS_EResult EOSSDK_Sessions::CopySessionHandleByInviteId(const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    
    if (Options == nullptr || Options->InviteId == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto it = std::find_if(_session_invites.begin(), _session_invites.end(), [Options](session_invite_t const& invite)
    {
        return invite.invite_id == Options->InviteId;
    });

    if (it == _session_invites.end())
        return EOS_EResult::EOS_NotFound;
    
    EOSSDK_SessionDetails* details = new EOSSDK_SessionDetails;

    details->_infos = it->infos;
    *OutSessionHandle = reinterpret_cast<EOS_HSessionDetails>(details);

    return EOS_EResult::EOS_Success;
}

/**
 * EOS_Sessions_CopySessionHandleForPresence is used to immediately retrieve a handle to the session information which was marked with bPresenceEnabled on create or join.
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid invite id or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if there is no session with bPresenceEnabled
 *
 * @see EOS_Sessions_CopySessionHandleForPresenceOptions
 * @see EOS_SessionDetails_Release
 */
EOS_EResult EOSSDK_Sessions::CopySessionHandleForPresence(const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    for (auto const& session : _sessions)
    {
        if (session.second.infos.presence_allowed())
        {
            EOSSDK_SessionDetails *details = new EOSSDK_SessionDetails;
            details->_infos = session.second.infos;
            *OutSessionHandle = reinterpret_cast<EOS_HSessionDetails>(details);
            return EOS_EResult::EOS_Success;
        }
    }

    *OutSessionHandle = nullptr;
    return EOS_EResult::EOS_NotFound;
}

/**
 * EOS_Sessions_IsUserInSession returns whether or not a given user can be found in a specified session
 *
 * @param Options Structure containing the input parameters
 *
 * @return EOS_Success if the user is found in the specified session
 *		   EOS_NotFound if the user is not found in the specified session
 *		   EOS_InvalidParameters if you pass an invalid invite id or a null pointer for the out parameter
 *		   EOS_IncompatibleVersion if the API version passed in is incorrect
 *		   EOS_Invalid_ProductUserID if an invalid target user is specified
 *		   EOS_Sessions_InvalidSession if the session specified is invalid
 */
EOS_EResult EOSSDK_Sessions::IsUserInSession(const EOS_Sessions_IsUserInSessionOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->TargetUserId == nullptr || Options->SessionName == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (Options->TargetUserId == GetEOS_Connect().product_id())
    {
        auto session = get_session_by_name(Options->SessionName);
        if (session != nullptr)
        {
            for (auto const& player : session->infos.players())
            {
                if (GetProductUserId(player) == Options->TargetUserId)
                {
                    return EOS_EResult::EOS_Success;
                }
            }
        }
        else
        {
            return EOS_EResult::EOS_Sessions_InvalidSession;
        }
    }
    else
    {
        auto user_infos = GetEOS_Connect().get_user_by_productid(Options->TargetUserId);
        if (user_infos != nullptr)
        {
            for (auto const& session : user_infos->second.infos.sessions())
            {
                if (session.first == Options->SessionName)
                {
                    return EOS_EResult::EOS_Success;
                }
            }
        }
    }

    return EOS_EResult::EOS_NotFound;
}

/**
 * Dump the contents of active sessions that exist locally to the log output, purely for debug purposes
 *
 * @param Options Options related to dumping session state such as the session name
 *
 * @return EOS_Success if the output operation completes successfully
 *         EOS_NotFound if the session specified does not exist
 *         EOS_InvalidParameters if any of the options are incorrect
 */

EOS_EResult EOSSDK_Sessions::DumpSessionState(const EOS_Sessions_DumpSessionStateOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Sessions::send_to_all_members(Network_Message_pb & msg, session_state_t* session)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    assert(session != nullptr);

    msg.set_source_id(user_id);
    for (auto const& player : session->infos.players())
    {
        if (player != user_id)
        {
            msg.set_dest_id(player);
            GetNetwork().TCPSendTo(msg);
        }
    }
    return true;
}

bool EOSSDK_Sessions::send_session_info_request(Network::peer_t const& peerid, Session_Info_Request_pb* req)
{
    TRACE_FUNC();
    // TODO: Make it P2P, send it to all, will have to filter results
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_sessions_request(req);

    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_info(session_state_t* session)
{
    TRACE_FUNC();
    if (session == nullptr)
        return false;

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;

    session_pb->set_allocated_session_info(&session->infos);
    msg.set_allocated_session(session_pb);

    bool res = send_to_all_members(msg, session);;

    session_pb->release_session_info();

    return res;
}

bool EOSSDK_Sessions::send_session_destroy(session_state_t *session)
{
    TRACE_FUNC();
    if (session == nullptr)
        return false;

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;
    Session_Destroy_pb* destr = new Session_Destroy_pb;

    destr->set_sessionid(session->infos.sessionid());
    destr->set_sessionname(session->infos.sessionname());

    session_pb->set_allocated_session_destroy(destr);
    msg.set_allocated_session(session_pb);

    return send_to_all_members(msg, session);
}

bool EOSSDK_Sessions::send_sessions_search_response(Network::peer_t const& peerid, Sessions_Search_response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Sessions_Search_Message_pb* search = new Sessions_Search_Message_pb;

    search->set_allocated_search_response(resp);
    msg.set_allocated_sessions_search(search);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_join_request(session_state_t *session)
{
    TRACE_FUNC();
    if (session == nullptr)
        return false;

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;
    Session_Join_Request_pb* req = new Session_Join_Request_pb;

    session_pb->set_allocated_session_join_request(req);
    msg.set_allocated_session(session_pb);

    req->set_sessionid(session->infos.sessionid());
    req->set_sessionname(session->infos.sessionname());

    return send_to_all_members(msg, session);
}

bool EOSSDK_Sessions::send_session_join_response(Network::peer_t const& peerid, Session_Join_Response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_session_join_response(resp);
    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_invite(Network::peer_t const& peerid, Session_Invite_pb* invite)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_session_invite(invite);
    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_invite_response(Network::peer_t const& peerid, Session_Invite_Response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = GetEOS_Connect().product_id()->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_session_invite_response(resp);
    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);

    return GetNetwork().TCPSendTo(msg);
}


///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Sessions::on_session_info_request(Network_Message_pb const& msg, Session_Info_Request_pb const& req)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    auto session = get_session_by_id(req.sessionid());
    Session_Info_pb* infos;

    if (session == nullptr)
    {
        infos = new Session_Info_pb();
    }
    else
    {
        infos = new Session_Info_pb(session->infos);
    }

    Network_Message_pb resp;
    Session_Message_pb* session_pb = new Session_Message_pb;

    session_pb->set_allocated_session_info(infos);
    resp.set_allocated_session(session_pb);

    resp.set_source_id(GetEOS_Connect().product_id()->to_string());
    resp.set_dest_id(msg.source_id());

    return GetNetwork().TCPSendTo(resp);
}

bool EOSSDK_Sessions::on_session_info(Network_Message_pb const& msg, Session_Info_pb const& infos)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    session_state_t *session = get_session_by_name(infos.sessionname());
    if (session != nullptr)
        session->infos = infos;

    return true;
}

bool EOSSDK_Sessions::on_session_destroy(Network_Message_pb const& msg, Session_Destroy_pb const& destr)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    session_state_t* session = get_session_by_name(destr.sessionname());
    if (session != nullptr)
        remove_player_from_session(msg.source_id(), get_session_by_name(destr.sessionname()));

    return true;
}

bool EOSSDK_Sessions::on_sessions_search(Network_Message_pb const& msg, Sessions_Search_pb const& search)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    Sessions_Search_response_pb* resp = new Sessions_Search_response_pb;
    resp->set_search_id(search.search_id());

    if (search.parameters_size() > 0)
    {
        session_state_t* session = get_session_from_attributes(search.parameters());
        if (session == nullptr)
        {
            resp->set_allocated_session_infos(new Session_Info_pb);
        }
        else
        {
            resp->set_allocated_session_infos(new Session_Info_pb(session->infos));
        }
    }
    else
    {
        auto it = _sessions.find(search.sessionid());
        if (it == _sessions.end())
        {
            resp->set_allocated_session_infos(new Session_Info_pb);
        }
        else
        {
            resp->set_allocated_session_infos(new Session_Info_pb(it->second.infos));
        }
    }

    return send_sessions_search_response(msg.source_id(), resp);
}

bool EOSSDK_Sessions::on_session_join_request(Network_Message_pb const& msg, Session_Join_Request_pb const& req)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (!is_player_registered(GetEOS_Connect().product_id()->to_string(), get_session_by_name(req.sessionname())))
    {// We are not in the session or we are not registered, we cannot accept the session join
        return true;
    }

    Session_Join_Response_pb* resp = new Session_Join_Response_pb;

    resp->set_sessionname(req.sessionname());
    resp->set_user_id(msg.source_id());

    // If we know the user
    if (GetEOS_Connect().get_user_by_productid(GetProductUserId(msg.source_id())) != nullptr)
    {
        auto it = _sessions.find(req.sessionname());
        if (it != _sessions.end())
        {
            if (it->second.infos.maxplayers() - it->second.infos.players_size())
            {
                resp->set_reason(get_enum_value(EOS_EResult::EOS_Success));
                *it->second.infos.add_players() = msg.source_id();
            }
            else
            {
                resp->set_reason(get_enum_value(EOS_EResult::EOS_Sessions_TooManyPlayers));
            }
        }
        else
        {
            resp->set_reason(get_enum_value(EOS_EResult::EOS_NotFound));
        }
    }
    else
    {
        resp->set_reason(get_enum_value(EOS_EResult::EOS_Sessions_NotAllowed));
    }

    return send_session_join_response(msg.source_id(), resp);
}

bool EOSSDK_Sessions::on_session_join_response(Network_Message_pb const& msg, Session_Join_Response_pb const& resp)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    auto reason = static_cast<EOS_EResult>(resp.reason());
    if (resp.user_id() == GetEOS_Connect().product_id()->to_string())
    {
        auto it = _sessions_join.find(resp.sessionname());
        if (it != _sessions_join.end())
        {
            EOS_Sessions_JoinSessionCallbackInfo& jsci = it->second->GetCallback<EOS_Sessions_JoinSessionCallbackInfo>();
            jsci.ResultCode = static_cast<EOS_EResult>(resp.reason());
            it->second->done = true;
        }
    }
    else
    {// We are not joining, so someone else is joining
        if (reason == EOS_EResult::EOS_Success)
        {// If the user has been accepted in the session
            add_player_to_session(resp.user_id(), get_session_by_name(resp.sessionname()));
        }
    }

    return true;
}

bool EOSSDK_Sessions::on_session_invite(Network_Message_pb const& msg, Session_Invite_pb const& invite)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(this, EOS_Sessions_SessionInviteReceivedCallbackInfo::k_iCallback));
    for (auto& notif : notifs)
    {
        EOS_Sessions_SessionInviteReceivedCallbackInfo& sirci = notif->GetCallback<EOS_Sessions_SessionInviteReceivedCallbackInfo>();
        
        auto invite_id(generate_account_id());
        strncpy(const_cast<char*>(sirci.InviteId), invite_id.c_str(), 65);
        sirci.TargetUserId = GetProductUserId(msg.source_id());

        session_invite_t invite_infos;
        invite_infos.infos = invite.infos();
        invite_infos.invite_id = std::move(invite_id);
        invite_infos.peer_id = sirci.TargetUserId;
        _session_invites.emplace_back(std::move(invite_infos));
        notif->res.cb_func(notif->res.data);
    }

    return true;
}

bool EOSSDK_Sessions::on_session_invite_response(Network_Message_pb const& msg, Session_Invite_Response_pb const& resp)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(this, EOS_Sessions_SessionInviteAcceptedCallbackInfo::k_iCallback));
    for (auto& notif : notifs)
    {
        EOS_Sessions_SessionInviteAcceptedCallbackInfo& siacbi = notif->GetCallback<EOS_Sessions_SessionInviteAcceptedCallbackInfo>();

        siacbi.TargetUserId = GetProductUserId(msg.source_id());
        strncpy(const_cast<char*>(siacbi.SessionId), resp.sessionid().c_str(), 65);

        notif->res.cb_func(notif->res.data);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Sessions::CBRunFrame()
{
    GLOBAL_LOCK();

    for (auto it = _session_searchs.begin(); it != _session_searchs.end();)
    {
        if ((*it)->released())
        {
            delete *it;
            it = _session_searchs.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return true;
}

bool EOSSDK_Sessions::RunNetwork(Network_Message_pb const& msg)
{
    switch (msg.messages_case())
    {
        case Network_Message_pb::MessagesCase::kSession:
        {
            if (GetEpicUserId(msg.source_id()) == Settings::Inst().userid)
                return true;

            Session_Message_pb const& session = msg.session();

            switch (session.message_case())
            {
                case Session_Message_pb::MessageCase::kSessionsRequest      : return on_session_info_request(msg, session.sessions_request());
                case Session_Message_pb::MessageCase::kSessionInfo          : return on_session_info(msg, session.session_info());
                case Session_Message_pb::MessageCase::kSessionDestroy       : return on_session_destroy(msg, session.session_destroy());
                case Session_Message_pb::MessageCase::kSessionJoinRequest   : return on_session_join_request(msg, session.session_join_request());
                case Session_Message_pb::MessageCase::kSessionJoinResponse  : return on_session_join_response(msg, session.session_join_response());
                case Session_Message_pb::MessageCase::kSessionInvite        : return on_session_invite(msg, session.session_invite());
                case Session_Message_pb::MessageCase::kSessionInviteResponse: return on_session_invite_response(msg, session.session_invite_response());
                default: LOG(Log::LogLevel::WARN, "Unhandled network message %d", session.message_case());
            }
        }
        break;

        case Network_Message_pb::MessagesCase::kSessionsSearch:
        {
            Sessions_Search_Message_pb const& search = msg.sessions_search();

            switch (search.message_case())
            {
                case Sessions_Search_Message_pb::MessageCase::kSearch: return on_sessions_search(msg, search.search());
            }
        }
    }
    

    return true;
}

bool EOSSDK_Sessions::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        case EOS_Sessions_JoinSessionCallbackInfo::k_iCallback:
        {
            auto now = std::chrono::steady_clock::now();
            if ((now - res->created_time) > join_timeout)
            {
                EOS_Sessions_JoinSessionCallbackInfo& jsci = res->GetCallback<EOS_Sessions_JoinSessionCallbackInfo>();
                jsci.ResultCode = EOS_EResult::EOS_TimedOut;

                auto it = std::find_if(_sessions_join.begin(), _sessions_join.end(), [&res]( std::pair<std::string const, pFrameResult_t> &join )
                {
                    return res == join.second;
                });
                if (it != _sessions_join.end())
                    _sessions_join.erase(it);

                res->done = true;
            }
        }
        break;
    }

    return res->done;
}

void EOSSDK_Sessions::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        case EOS_Sessions_UpdateSessionCallbackInfo::k_iCallback:
        {
            EOS_Sessions_UpdateSessionCallbackInfo& usci = res->GetCallback<EOS_Sessions_UpdateSessionCallbackInfo>();
            delete[]usci.SessionId;
            delete[]usci.SessionName;
        }
        break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        case EOS_Sessions_SessionInviteReceivedCallbackInfo::k_iCallback:
        {
            EOS_Sessions_SessionInviteReceivedCallbackInfo& callback = res->GetCallback<EOS_Sessions_SessionInviteReceivedCallbackInfo>();
            // Free resources
            delete[]callback.InviteId;
        }
        break;
        case EOS_Sessions_SessionInviteAcceptedCallbackInfo::k_iCallback:
        {
            EOS_Sessions_SessionInviteAcceptedCallbackInfo& callback = res->GetCallback<EOS_Sessions_SessionInviteAcceptedCallbackInfo>();
            // Free resources
            delete[]callback.SessionId;
        }
        break;
        
    }
}

}