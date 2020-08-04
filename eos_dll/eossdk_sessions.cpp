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

session_state_t* EOSSDK_Sessions::get_session_by_id(std::string const& session_id)
{
    auto it = std::find_if(_sessions.begin(), _sessions.end(), [&session_id]( std::pair<std::string const, session_state_t>& infos)
    {
        return session_id == infos.second.infos.session_id();
    });
    if (it == _sessions.end())
        return nullptr;

    return &it->second;
}

session_state_t* EOSSDK_Sessions::get_session_by_name(std::string const& session_name)
{
    auto it = _sessions.find(session_name);
    if (it == _sessions.end())
        return nullptr;

    return &it->second;
}

std::vector<session_state_t*> EOSSDK_Sessions::get_sessions_from_attributes(google::protobuf::Map<std::string, Session_Search_Parameter> const& parameters)
{
    std::vector<session_state_t*> res;
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
                        std::string const& s_session = session.second.infos.bucket_id();
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
            res.emplace_back(&session.second);
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

bool EOSSDK_Sessions::register_player_to_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr && !is_player_registered(player, session))
    {
        *session->infos.add_registered_players() = player;
        return true;
    }

    return false;
}

bool EOSSDK_Sessions::unregister_player_from_session(std::string const& player, session_state_t* session)
{
    if (session != nullptr)
    {
        auto it = std::find(session->infos.registered_players().begin(), session->infos.registered_players().end(), player);
        if (it != session->infos.registered_players().end())
        {
            session->infos.mutable_registered_players()->erase(it);
            return true;
        }
    }

    return false;
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
 * The Session Interface is used to manage sessions that can be advertised with the backend service
 * All Session Interface calls take a handle of type EOS_HSessions as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetSessionsInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetSessionsInterface
 */

/**
  * Creates a session modification handle (EOS_HSessionModification).  The session modification handle is used to build a new session and can be applied with EOS_Sessions_UpdateSession
  * The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it no longer needed.
  *
  * @param Options Required fields for the creation of a session such as a name, bucket_id, and max players
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

    if (Options == nullptr || Options->SessionName == nullptr || Options->BucketId == nullptr || OutSessionModificationHandle == nullptr)
    {
        set_nullptr(OutSessionModificationHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOSSDK_SessionModification* modif = new EOSSDK_SessionModification;
    modif->_api_version = Options->ApiVersion;
    modif->_type = EOSSDK_SessionModification::modif_type::creation;
    modif->_infos.set_host_address("127.0.0.1");

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
            modif->_infos.set_bucket_id(opts->BucketId);
            modif->_infos.set_max_players(opts->MaxPlayers);
            modif->_session_name = opts->SessionName;

            LOG(Log::LogLevel::DEBUG, "Starting session creation: session_name = %s, bucket_id = %s, presence_enabled: %d", modif->_session_name.c_str(), modif->_infos.bucket_id().c_str(), (int)modif->_infos.presence_allowed());
        }
        break;

        default:
            LOG(Log::LogLevel::FATAL, "Unmanaged API version %d", Options->ApiVersion);
            abort();
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

    if (Options == nullptr || Options->SessionName == nullptr || OutSessionModificationHandle == nullptr)
    {
        set_nullptr(OutSessionModificationHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOSSDK_SessionModification* modif = new EOSSDK_SessionModification;
    modif->_api_version = Options->ApiVersion;
    modif->_type = EOSSDK_SessionModification::modif_type::update;

    session_state_t* session = get_session_by_name(Options->SessionName);
    modif->_session_name = Options->SessionName;

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
                LOG(Log::LogLevel::DEBUG, "Starting session modification: session_name = %s", modif->_session_name.c_str());
            }
            break;

            default:
                LOG(Log::LogLevel::FATAL, "Unmanaged API version %d", Options->ApiVersion);
                abort();
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
            *str = '\0';
            usci.SessionName = str;
        }
    }
    else
    {
        EOSSDK_SessionModification* modif = reinterpret_cast<EOSSDK_SessionModification*>(Options->SessionModificationHandle);
        usci.SessionId = nullptr;
        {
            std::string const& sess_name = modif->_session_name;
            char* name = new char[sess_name.length() + 1];
            strncpy(name, sess_name.c_str(), sess_name.length() + 1);
            usci.SessionName = name;
        }
        session_state_t* session = get_session_by_id(modif->_infos.session_id());

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
                    auto& session = _sessions[modif->_session_name];
                    {
                        modif->_infos.set_session_id(generate_account_id());

                        std::string const& sess_id = modif->_infos.session_id();
                        char* session_id = new char[sess_id.length() + 1];
                        strncpy(session_id, sess_id.c_str(), sess_id.length() + 1);
                        usci.SessionId = session_id;
                    }
                    session.state = session_state_t::state_e::created;
                    session.infos = modif->_infos;

                    LOG(Log::LogLevel::DEBUG, "Session created: \n"
                        "  session_name: %s\n"
                        "  session_id: %s\n"
                        "  bucket_id: %s\n"
                        "  host_address: %s\n",
                        modif->_session_name.c_str(),
                        modif->_infos.session_id().c_str(),
                        modif->_infos.bucket_id().c_str(),
                        modif->_infos.host_address().c_str()
                    );

                    session.infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Pending));
                    *session.infos.add_players() = Settings::Inst().productuserid->to_string();
                    *session.infos.add_registered_players() = Settings::Inst().productuserid->to_string();
                    //GetEOS_Connect().add_session(GetProductUserId(session.infos.session_id()), session.infos.session_name());

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
                    modif->_infos.set_session_id(session->infos.session_id());
                    modif->_infos.set_state(session->infos.state());
                    session->infos = modif->_infos;
                    {
                        std::string const& sess_id = session->infos.session_id();
                        char* session_id = new char[sess_id.length() + 1];
                        strncpy(session_id, sess_id.c_str(), sess_id.length() + 1);
                        usci.SessionId = session_id;
                    }

                    LOG(Log::LogLevel::DEBUG, "Session modified: \n"
                        "  session_name: %s\n"
                        "  session_id: %s\n"
                        "  bucket_id: %s\n"
                        "  host_address: %s\n",
                        modif->_session_name.c_str(),
                        modif->_infos.session_id().c_str(),
                        modif->_infos.bucket_id().c_str(),
                        modif->_infos.host_address().c_str()
                    );

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
            LOG(Log::LogLevel::DEBUG, "Destroying session: name %s", Options->SessionName);

            dsci.ResultCode = EOS_EResult::EOS_Success;

            auto join_it = _sessions_join.find(Options->SessionName);
            if (join_it != _sessions_join.end())
            {
                EOS_Sessions_JoinSessionCallbackInfo& jsci = join_it->second->GetCallback<EOS_Sessions_JoinSessionCallbackInfo>();
                jsci.ResultCode = EOS_EResult::EOS_UnexpectedError;
                res->done = true;
                _sessions_join.erase(join_it);
            }

            it->second.infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_Destroying));

            send_session_destroy(&it->second);
            //GetEOS_Connect().remove_session(GetProductUserId(it->second.infos.session_id()), it->second.infos.session_name());
            _sessions.erase(it);
        }
        else
        {
            LOG(Log::LogLevel::DEBUG, "Destroying session: name %s Not Found", Options->SessionName);

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
        EOSSDK_SessionDetails* details = reinterpret_cast<EOSSDK_SessionDetails*>(Options->SessionHandle);
        if (_sessions.count(Options->SessionName) == 0) // If we haven't already a session with that name (created, joining or joined)
        {
            LOG(Log::LogLevel::DEBUG, "Joining session: name %s, session_id: %s", Options->SessionName, details->_infos.session_id().c_str());

            switch ((EOS_EOnlineSessionState)details->_infos.state())
            {
                case EOS_EOnlineSessionState::EOS_OSS_InProgress:
                    if (!details->_infos.join_in_progress_allowed())
                    {
                        //jsci.ResultCode = EOS_EResult::EOS_Sessions_SessionInProgress;
                        jsci.ResultCode = EOS_EResult::EOS_Sessions_NotAllowed;
                        res->done = true;
                        break;
                    }
                    // Allowed to join while in progress
                
                case EOS_EOnlineSessionState::EOS_OSS_Pending   :
                {
                    Session_Join_Request_pb* join = new Session_Join_Request_pb;
                    join->set_session_id(details->_infos.session_id());

                    session_state_t& session = _sessions[Options->SessionName];
                    session.state = session_state_t::state_e::joining;
                    session.infos = details->_infos;
                    _sessions_join[details->_infos.session_id()] = res;

                    jsci.ResultCode = EOS_EResult::EOS_UnexpectedError;
                    send_session_join_request(&session);
                }
                break;

                default:
                {
                    jsci.ResultCode = EOS_EResult::EOS_Sessions_NotAllowed;
                    res->done = true;
                }
            }
        }
        else
        {
            LOG(Log::LogLevel::DEBUG, "joining session: name %s Already Exists, session_id: %s", Options->SessionName, details->_infos.session_id().c_str());

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
        session_state_t* session = get_session_by_name(Options->SessionName);
        if (session != nullptr)
        {
            LOG(Log::LogLevel::DEBUG, "Starting session: name %s", Options->SessionName);

            ssci.ResultCode = EOS_EResult::EOS_Success;
            switch ((EOS_EOnlineSessionState)session->infos.state())
            {
                case EOS_EOnlineSessionState::EOS_OSS_Destroying:
                case EOS_EOnlineSessionState::EOS_OSS_NoSession :
                case EOS_EOnlineSessionState::EOS_OSS_Ending    :
                case EOS_EOnlineSessionState::EOS_OSS_Creating  :
                case EOS_EOnlineSessionState::EOS_OSS_Starting  :
                case EOS_EOnlineSessionState::EOS_OSS_InProgress:
                    ssci.ResultCode = EOS_EResult::EOS_InvalidParameters;
                    break;

                case EOS_EOnlineSessionState::EOS_OSS_Ended     :
                case EOS_EOnlineSessionState::EOS_OSS_Pending   :
                    session->infos.set_state(get_enum_value(EOS_EOnlineSessionState::EOS_OSS_InProgress));
                    send_session_info(session);
            }
        }
        else
        {
            LOG(Log::LogLevel::DEBUG, "Starting session: name %s Not Found", Options->SessionName);
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
        session_state_t* session = get_session_by_name(Options->SessionName);
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
            if (is_player_registered(Settings::Inst().productuserid->to_string(), session))
            {
                google::protobuf::RepeatedPtrField<std::string> registered;
                for (uint32_t i = 0; i < Options->PlayersToRegisterCount; ++i)
                {
                    if (register_player_to_session(Options->PlayersToRegister[i]->to_string(), session))
                    {
                        *registered.Add() = Options->PlayersToRegister[i]->to_string();
                    }
                }
                if (registered.empty())
                {
                    rpci.ResultCode = EOS_EResult::EOS_NoChange;
                }
                else
                {
                    rpci.ResultCode = EOS_EResult::EOS_Success;

                    std::string const& user_id = Settings::Inst().productuserid->to_string();                    

                    Session_Register_pb* register_ = new Session_Register_pb;

                    register_->set_session_id(session->infos.session_id());
                    *register_->mutable_member_ids() = std::move(registered);

                    send_session_register(register_, session);
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
            if (is_player_registered(Settings::Inst().productuserid->to_string(), session))
            {
                google::protobuf::RepeatedPtrField<std::string> unregistered;
                for (uint32_t i = 0; i < Options->PlayersToUnregisterCount; ++i)
                {
                    if (unregister_player_from_session(Options->PlayersToUnregister[i]->to_string(), session))
                    {
                        *unregistered.Add() = Options->PlayersToUnregister[i]->to_string();
                    }
                }
                if (unregistered.empty())
                {
                    upci.ResultCode = EOS_EResult::EOS_NoChange;
                }
                else
                {
                    upci.ResultCode = EOS_EResult::EOS_Success;

                    std::string const& user_id = Settings::Inst().productuserid->to_string();

                    Session_Unregister_pb* unregister = new Session_Unregister_pb;

                    unregister->set_session_id(session->infos.session_id());
                    *unregister->mutable_member_ids() = std::move(unregistered);

                    send_session_unregister(unregister, session);
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
        session_state_t* session = get_session_by_name(Options->SessionName);
        if (session == nullptr || GetEOS_Connect().get_user_by_productid(Options->TargetUserId) == GetEOS_Connect().get_end_users())
        {
            sici.ResultCode = EOS_EResult::EOS_NotFound;
        }
        else
        {
            Session_Invite_pb* invite = new Session_Invite_pb;
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

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_RejectInviteCallbackInfo& rici = res->CreateCallback<EOS_Sessions_RejectInviteCallbackInfo>((CallbackFunc)CompletionDelegate);

    rici.ClientData = ClientData;

    auto it = std::find_if(_session_invites.begin(), _session_invites.end(), [Options]( session_invite_t& invite)
    {
        return invite.invite_id == Options->InviteId;
    });

    if (it == _session_invites.end())
    {
        rici.ResultCode = EOS_EResult::EOS_NotFound;
    }
    else
    {
        rici.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
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

    if (Options == nullptr || Options->MaxSearchResults == 0 || OutSessionSearchHandle == nullptr)
    {
        set_nullptr(OutSessionSearchHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }
    
    _session_searchs.emplace_back();
    EOSSDK_SessionSearch*& session_search = _session_searchs.back();
    session_search = new EOSSDK_SessionSearch;

    *OutSessionSearchHandle = reinterpret_cast<EOS_HSessionSearch>(session_search);

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

    if (Options == nullptr || Options->SessionName == nullptr || OutSessionHandle == nullptr)
    {
        set_nullptr(OutSessionHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }

    session_state_t* session = get_session_by_name(Options->SessionName);
    if (session == nullptr)
    {
        LOG(Log::LogLevel::DEBUG, "Didn't find Active Session %s", Options->SessionName);
        return EOS_EResult::EOS_NotFound;
    }

    LOG(Log::LogLevel::DEBUG, "Found Active Session %s", Options->SessionName);
    EOSSDK_ActiveSession* active_session = new EOSSDK_ActiveSession;
    
    active_session->_session_name = Options->SessionName;
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
    sirci.InviteId = new char[max_id_length];

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
    siaci.SessionId = new char[max_id_length];

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
 * Register to receive notifications when a user accepts a session join game via the social overlay.
 * @note must call RemoveNotifyJoinSessionAccepted to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Sessions::AddNotifyJoinSessionAccepted(const EOS_Sessions_AddNotifyJoinSessionAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionAcceptedCallback NotificationFn)
{
    TRACE_FUNC();

    if (NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);

    EOS_Sessions_JoinSessionAcceptedCallbackInfo& jsaci = res->CreateCallback<EOS_Sessions_JoinSessionAcceptedCallbackInfo>((CallbackFunc)NotificationFn);

    jsaci.ClientData = ClientData;
    jsaci.LocalUserId = GetEOS_Connect().get_myself()->first;
    jsaci.UiEventId = EOS_UI_EVENTID_INVALID;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when a user accepts a session join game via the social overlay.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Sessions::RemoveNotifyJoinSessionAccepted(EOS_NotificationId InId)
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
    
    if (Options == nullptr || Options->InviteId == nullptr || OutSessionHandle == nullptr)
    {
        set_nullptr(OutSessionHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = std::find_if(_session_invites.begin(), _session_invites.end(), [Options](session_invite_t const& invite)
    {
        return invite.invite_id == Options->InviteId;
    });

    if (it == _session_invites.end())
    {
        return EOS_EResult::EOS_NotFound;
    }
    
    EOSSDK_SessionDetails* details = new EOSSDK_SessionDetails;

    details->_infos = it->infos;

    *OutSessionHandle = reinterpret_cast<EOS_HSessionDetails>(details);

    return EOS_EResult::EOS_Success;
}

/**
 * EOS_Sessions_CopySessionHandleByUiEventId is used to immediately retrieve a handle to the session information from after notification of a join game event.
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
 * @see EOS_Sessions_CopySessionHandleByUiEventIdOptions
 * @see EOS_SessionDetails_Release
 */
EOS_EResult EOSSDK_Sessions::CopySessionHandleByUiEventId(const EOS_Sessions_CopySessionHandleByUiEventIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    TRACE_FUNC();
    GLOBAL_LOCK();
    
    if (Options == nullptr || Options->UiEventId == EOS_UI_EVENTID_INVALID || OutSessionHandle == nullptr)
    {
        set_nullptr(OutSessionHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }

    *OutSessionHandle = nullptr;
    return EOS_EResult::EOS_InvalidParameters;
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

    if (Options == nullptr || OutSessionHandle == nullptr)
    {
        set_nullptr(OutSessionHandle);
        return EOS_EResult::EOS_InvalidParameters;
    }

    for (auto const& session : _sessions)
    {
        if (session.second.infos.presence_allowed())
        {
            LOG(Log::LogLevel::DEBUG, "Found Session for presence");

            EOSSDK_SessionDetails *details = new EOSSDK_SessionDetails;
            details->_infos = session.second.infos;
            *OutSessionHandle = reinterpret_cast<EOS_HSessionDetails>(details);
            return EOS_EResult::EOS_Success;
        }
    }

    LOG(Log::LogLevel::DEBUG, "Did not find Session for presence");
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

    if (Options->TargetUserId == Settings::Inst().productuserid)
    {
        session_state_t* session = get_session_by_name(Options->SessionName);
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
        if (user_infos != GetEOS_Connect().get_end_users())
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
    assert(session != nullptr);

    for (auto const& player : session->infos.players())
    {
        if (player != msg.source_id())
        {
            msg.set_dest_id(player);
            GetNetwork().TCPSendTo(msg);
        }
    }
    return true;
}

bool EOSSDK_Sessions::send_session_info_request(Network::peer_t const& peerid, Session_Infos_Request_pb* req)
{
    TRACE_FUNC();
    // TODO: Make it P2P, send it to all, will have to filter results
    std::string const& user_id = Settings::Inst().productuserid->to_string();

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
    assert(session != nullptr);
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;

    session_pb->set_allocated_session_infos(&session->infos);
    msg.set_allocated_session(session_pb);
    msg.set_source_id(user_id);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    bool res = send_to_all_members(msg, session);;

    session_pb->release_session_infos();

    return res;
}

bool EOSSDK_Sessions::send_session_destroy(session_state_t *session)
{
    TRACE_FUNC();
    assert(session != nullptr);
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;
    Session_Destroy_pb* destr = new Session_Destroy_pb;

    destr->set_session_id(session->infos.session_id());

    session_pb->set_allocated_session_destroy(destr);
    msg.set_allocated_session(session_pb);
    msg.set_source_id(user_id);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    return send_to_all_members(msg, session);
}

bool EOSSDK_Sessions::send_sessions_search_response(Network::peer_t const& peerid, Sessions_Search_response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Sessions_Search_Message_pb* search = new Sessions_Search_Message_pb;

    search->set_allocated_search_response(resp);
    msg.set_allocated_sessions_search(search);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_join_request(session_state_t *session)
{
    TRACE_FUNC();
    assert(session != nullptr);
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;
    Session_Join_Request_pb* req = new Session_Join_Request_pb;

    session_pb->set_allocated_session_join_request(req);
    msg.set_allocated_session(session_pb);
    msg.set_source_id(user_id);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    req->set_session_id(session->infos.session_id());

    return send_to_all_members(msg, session);
}

bool EOSSDK_Sessions::send_session_join_response(Network::peer_t const& peerid, Session_Join_Response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_session_join_response(resp);
    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    session_state_t* pSession = get_session_by_id(resp->session_id());

    if (pSession != nullptr)
    {// Notify all session members of a join status
        send_to_all_members(msg, pSession);
    }
    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_invite(Network::peer_t const& peerid, Session_Invite_pb* invite)
{
    TRACE_FUNC();
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_session_invite(invite);
    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_invite_response(Network::peer_t const& peerid, Session_Invite_Response_pb* resp)
{
    TRACE_FUNC();
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session = new Session_Message_pb;

    session->set_allocated_session_invite_response(resp);
    msg.set_allocated_session(session);

    msg.set_source_id(user_id);
    msg.set_dest_id(peerid);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    return GetNetwork().TCPSendTo(msg);
}

bool EOSSDK_Sessions::send_session_register(Session_Register_pb* register_, session_state_t* session)
{
    TRACE_FUNC();
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;

    session_pb->set_allocated_session_register(register_);
    msg.set_allocated_session(session_pb);

    msg.set_source_id(user_id);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    return send_to_all_members(msg, session);
}

bool EOSSDK_Sessions::send_session_unregister(Session_Unregister_pb* unregister, session_state_t* session)
{
    TRACE_FUNC();
    std::string const& user_id = Settings::Inst().productuserid->to_string();

    Network_Message_pb msg;
    Session_Message_pb* session_pb = new Session_Message_pb;

    session_pb->set_allocated_session_unregister(unregister);
    msg.set_allocated_session(session_pb);

    msg.set_source_id(user_id);
    msg.set_game_id(EOSSDK_Client::Inst()._product_name);

    return send_to_all_members(msg, session);
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Sessions::on_peer_disconnect(Network_Message_pb const& msg, Network_Peer_Disconnect_pb const& peer)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    for (auto& session : _sessions)
    {
        remove_player_from_session(msg.source_id(), &session.second);
    }

    return true;
}

bool EOSSDK_Sessions::on_session_info_request(Network_Message_pb const& msg, Session_Infos_Request_pb const& req)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    session_state_t* session = get_session_by_id(req.session_id());
    Session_Infos_pb* infos;

    if (session == nullptr)
    {
        infos = new Session_Infos_pb();
    }
    else
    {
        infos = new Session_Infos_pb(session->infos);
    }

    Network_Message_pb resp;
    Session_Message_pb* session_pb = new Session_Message_pb;

    session_pb->set_allocated_session_infos(infos);
    resp.set_allocated_session(session_pb);

    resp.set_source_id(Settings::Inst().productuserid->to_string());
    resp.set_dest_id(msg.source_id());

    return GetNetwork().TCPSendTo(resp);
}

bool EOSSDK_Sessions::on_session_info(Network_Message_pb const& msg, Session_Infos_pb const& infos)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    session_state_t *session = get_session_by_id(infos.session_id());
    if (session != nullptr)
        session->infos = infos;

    return true;
}

bool EOSSDK_Sessions::on_session_destroy(Network_Message_pb const& msg, Session_Destroy_pb const& destr)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    session_state_t* session = get_session_by_id(destr.session_id());
    if (session != nullptr)
        remove_player_from_session(msg.source_id(), session);

    return true;
}

bool EOSSDK_Sessions::on_sessions_search(Network_Message_pb const& msg, Sessions_Search_pb const& search)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    Sessions_Search_response_pb* resp = new Sessions_Search_response_pb;
    resp->set_search_id(search.search_id());

    if (msg.game_id() == EOSSDK_Client::Inst()._product_name)
    {
        if (search.parameters_size() > 0)
        {
            std::vector<session_state_t*> sessions = std::move(get_sessions_from_attributes(search.parameters()));
            for (auto& session : sessions)
            {
                *resp->mutable_sessions()->Add() = session->infos;
            }
        }
        else if (!search.session_id().empty())
        {
            session_state_t* pSession = get_session_by_id(search.session_id());
            if (pSession != nullptr)
            {
                *resp->mutable_sessions()->Add() = pSession->infos;
            }
        }
        else if (GetProductUserId(search.target_id()) == GetEOS_Connect().get_myself()->first)
        {
            for (auto& session : _sessions)
            {
                *resp->mutable_sessions()->Add() = session.second.infos;
            }
        }
    }

    return send_sessions_search_response(msg.source_id(), resp);
}

bool EOSSDK_Sessions::on_session_join_request(Network_Message_pb const& msg, Session_Join_Request_pb const& req)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    session_state_t* pSession = get_session_by_id(req.session_id());
    if (!is_player_registered(Settings::Inst().productuserid->to_string(), pSession))
    {// We are not in the session or we are not registered, we cannot accept the session join
        return true;
    }

    Session_Join_Response_pb* resp = new Session_Join_Response_pb;

    resp->set_session_id(req.session_id());
    resp->set_user_id(msg.source_id());

    // If we know the user
    if (GetEOS_Connect().get_user_by_productid(GetProductUserId(msg.source_id())) != GetEOS_Connect().get_end_users())
    {
        if (pSession->infos.max_players() - pSession->infos.players_size())
        {
            LOG(Log::LogLevel::DEBUG, "Join request accepted.");
            resp->set_reason(get_enum_value(EOS_EResult::EOS_Success));
            add_player_to_session(msg.source_id(), pSession);
        }
        else
        {
            LOG(Log::LogLevel::DEBUG, "Join request rejected: This session is full.");
            resp->set_reason(get_enum_value(EOS_EResult::EOS_Sessions_TooManyPlayers));
        }
    }
    else
    {
        LOG(Log::LogLevel::DEBUG, "Join request rejected: We don't know (yet?) the user.");
        resp->set_reason(get_enum_value(EOS_EResult::EOS_Sessions_NotAllowed));
    }

    return send_session_join_response(msg.source_id(), resp);
}

bool EOSSDK_Sessions::on_session_join_response(Network_Message_pb const& msg, Session_Join_Response_pb const& resp)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    std::string const& user_id = Settings::Inst().productuserid->to_string();
    auto session_it = std::find_if(_sessions.begin(), _sessions.end(), [&resp]( std::pair<const std::string, session_state_t>& item)
    {
        return item.second.infos.session_id() == resp.session_id();
    });

    auto reason = static_cast<EOS_EResult>(resp.reason());
    if (resp.user_id() == Settings::Inst().productuserid->to_string())
    {
        auto it = _sessions_join.find(resp.session_id());
        if (it != _sessions_join.end())
        {
            EOS_Sessions_JoinSessionCallbackInfo& jsci = it->second->GetCallback<EOS_Sessions_JoinSessionCallbackInfo>();
            jsci.ResultCode = static_cast<EOS_EResult>(resp.reason());

            switch (jsci.ResultCode)
            {// Don't wait for a consensus, sessions are P2P, the first valid response is the right one
                case EOS_EResult::EOS_Sessions_NotAllowed:
                {// If this peer doesn't know us yet, set the error code but do not stop the join request, someone might accept us
                    LOG(Log::LogLevel::DEBUG, "(%s) Join request rejected: We don't know (yet?) the user.", msg.source_id().c_str());
                }
                break;

                case EOS_EResult::EOS_Sessions_TooManyPlayers:
                {
                    LOG(Log::LogLevel::DEBUG, "(%s) Join rejected: This session is full.", msg.source_id().c_str());
                    it->second->done = true;
                    _sessions_join.erase(it);
                    _sessions.erase(session_it);
                }
                break;
                
                case EOS_EResult::EOS_Success:
                {
                    LOG(Log::LogLevel::DEBUG, "(%s) Join accepted.", msg.source_id().c_str());
                    it->second->done = true;
                    _sessions_join.erase(it);
                    // Add myself to the session
                    //GetEOS_Connect().add_session(GetProductUserId(session_it->second.infos.session_id()), session_it->second.infos.session_name());

                    session_it->second.state = session_state_t::state_e::joined;
                    add_player_to_session(user_id, &session_it->second);
                }
                break;
            }
        }
        else
        {
            LOG(Log::LogLevel::DEBUG, "Join request not found.");
        }
    }
    else if(is_player_in_session(user_id, &session_it->second))
    {// We are not joining, so someone else is joining
        if (reason == EOS_EResult::EOS_Success)
        {// If the user has been accepted in the session
            LOG(Log::LogLevel::DEBUG, "Add new player (%s) to session.", resp.user_id().c_str());
            add_player_to_session(resp.user_id(), &session_it->second);
        }
    }

    return true;
}

bool EOSSDK_Sessions::on_session_invite(Network_Message_pb const& msg, Session_Invite_pb const& invite)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    EOS_ProductUserId target_id = GetProductUserId(msg.source_id());
    
    session_invite_t invite_infos;
    invite_infos.infos = invite.infos();
    invite_infos.invite_id = std::move(generate_account_id());
    invite_infos.peer_id = target_id;

    _session_invites.emplace_back(std::move(invite_infos));
    std::string const& invite_id = _session_invites.back().invite_id;

    std::vector<pFrameResult_t> notifs = std::move(GetCB_Manager().get_notifications(this, EOS_Sessions_SessionInviteReceivedCallbackInfo::k_iCallback));
    for (auto& notif : notifs)
    {
        EOS_Sessions_SessionInviteReceivedCallbackInfo& sirci = notif->GetCallback<EOS_Sessions_SessionInviteReceivedCallbackInfo>();
        strncpy(const_cast<char*>(sirci.InviteId), invite_id.c_str(), max_id_length);
        sirci.TargetUserId = target_id;

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
        strncpy(const_cast<char*>(siacbi.SessionId), resp.session_id().c_str(), max_id_length);

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
            delete* it;
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
            if (GetProductUserId(msg.source_id()) == GetEOS_Connect().get_myself()->first)
                return true;

            Session_Message_pb const& session = msg.session();

            switch (session.message_case())
            {
                case Session_Message_pb::MessageCase::kSessionsRequest      : return on_session_info_request(msg, session.sessions_request());
                case Session_Message_pb::MessageCase::kSessionInfos         : return on_session_info(msg, session.session_infos());
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
                if (jsci.ResultCode == EOS_EResult::EOS_UnexpectedError)
                {// If its the default error code, set the result code to TimedOut
                    jsci.ResultCode = EOS_EResult::EOS_TimedOut;
                }

                auto join_it = std::find_if(_sessions_join.begin(), _sessions_join.end(), [&res]( std::pair<std::string const, pFrameResult_t> &join )
                {
                    return res == join.second;
                });
                if (join_it != _sessions_join.end())
                {// We found the join callback
                    auto session_it = std::find_if(_sessions.begin(), _sessions.end(), [join_it]( std::pair<const std::string, session_state_t>& item)
                    {// Look if we can find the session
                        return item.second.infos.session_id() == join_it->first;
                    });
                    if (session_it == _sessions.end())
                    {// Session found, we got a timeout so remove it
                        _sessions.erase(session_it);
                    }

                    _sessions_join.erase(join_it);
                }

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