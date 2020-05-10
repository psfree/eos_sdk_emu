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

#include "eossdk_presence.h"
#include "eossdk_platform.h"
#include "eos_client_api.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_Presence::presence_query_timeout) EOSSDK_Presence::presence_query_timeout;

EOSSDK_Presence::EOSSDK_Presence()
{
    GetCB_Manager().register_callbacks(this);
    GetNetwork().register_listener(this, 0, Network_Message_pb::MessagesCase::kPresence);
}

EOSSDK_Presence::~EOSSDK_Presence()
{
    GetNetwork().unregister_listener(this, 0, Network_Message_pb::MessagesCase::kPresence);

    GetCB_Manager().remove_all_notifications(this);
    GetCB_Manager().unregister_callbacks(this);
}

void EOSSDK_Presence::setup_myself()
{
    auto& presence = get_myself();
    presence.set_userid(Settings::Inst().userid->to_string());
    presence.set_status(get_enum_value(EOS_Presence_EStatus::EOS_PS_Online));
    presence.set_productid(GetEOS_Platform()._product_id);
#if defined(__WINDOWS__)
    presence.set_platform("WIN");
#elif defined(__LINUX__)
    presence.set_platform("LINUX"); // TODO
#elif defined(__APPLE__)
    presence.set_platform("APPLE"); // TODO
#endif
}

Presence_Info_pb& EOSSDK_Presence::get_myself()
{
    return _presences[Settings::Inst().userid];
}

Presence_Info_pb* EOSSDK_Presence::get_presence(EOS_EpicAccountId userid)
{
    auto it = _presences.find(userid);
    if (it == _presences.end())
        return nullptr;

    return &it->second;
}

void EOSSDK_Presence::trigger_presence_change(EOS_EpicAccountId userid)
{
    auto notifs = std::move(GetCB_Manager().get_notifications(this, EOS_Presence_PresenceChangedCallbackInfo::k_iCallback));
    for (auto& notif : notifs)
    {
        auto& pcci = notif->GetCallback<EOS_Presence_PresenceChangedCallbackInfo>();
        pcci.PresenceUserId = userid;
        notif->res.cb_func(notif->res.data);
    }
}

void EOSSDK_Presence::set_user_status(EOS_EpicAccountId userid, EOS_Presence_EStatus status)
{
    auto& presence = _presences[userid];
    if (presence.status() != get_enum_value(status))
    {
        presence.set_status(get_enum_value(status));
        trigger_presence_change(userid);
    }
}

/**
 * Query a user's presence. This must complete successfully before CopyPresence will have valid results. If HasPresence returns true for a remote
 * user, this does not need to be called.
 *
 * @param Options Object containing properties related to who is querying presence and for what user
 * @param ClientData Optional pointer to help track this request, that is returned in the completion callback
 * @param CompletionDelegate Pointer to a function that handles receiving the completion information
 */
void EOSSDK_Presence::QueryPresence( const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);
    EOS_Presence_QueryPresenceCallbackInfo& qpci = res->CreateCallback<EOS_Presence_QueryPresenceCallbackInfo>((CallbackFunc)CompletionDelegate);
    qpci.ClientData = ClientData;
    qpci.LocalUserId = Settings::Inst().userid;

    if (Options == nullptr || Options->TargetUserId == nullptr)
    {
        qpci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        qpci.TargetUserId = GetEpicUserId(sdk::NULL_USER_ID);

        res->done = true;
        GetCB_Manager().add_callback(this, res);
        return;
    }

    qpci.TargetUserId = Options->TargetUserId;
    if (qpci.TargetUserId == Settings::Inst().userid)
    {
        qpci.ResultCode = EOS_EResult::EOS_Success;
        res->done = true;
    }
    else
    {
        auto* user = GetEOS_Connect().get_user_by_userid(Options->TargetUserId);
        if (user != nullptr)
        {
            _presence_queries[Options->TargetUserId].emplace_back(res);
            Presence_Info_Request_pb* req = new Presence_Info_Request_pb;
            send_presence_info_request(user->first->to_string(), req);
        }
        else
        {
            qpci.ResultCode = EOS_EResult::EOS_NotFound;
            res->done = true;
        }
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * Check if we already have presence for a user
 *
 * @param Options Object containing properties related to who is requesting presence and for what user
 * @return EOS_TRUE if we have presence for the requested user, or EOS_FALSE if the request was invalid or we do not have cached data
 */
EOS_Bool EOSSDK_Presence::HasPresence( const EOS_Presence_HasPresenceOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
    
    return (get_presence(Options->TargetUserId) == nullptr ? EOS_FALSE : EOS_TRUE);
}

/**
 * Get a user's cached presence object. If successful, this data must be released by calling EOS_Presence_Info_Release
 *
 * @param Options Object containing properties related to who is requesting presence and for what user
 * @param OutPresence A pointer to a pointer of Presence Info. If the returned result is success, this will be set to data that must be later released, otherwise this will be set to NULL
 * @return Success if we have cached data, or an error result if the request was invalid or we do not have cached data.
 *
 * @see EOS_Presence_Info_Release
 */
EOS_EResult EOSSDK_Presence::CopyPresence( const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence)
{
     // TODO: Check the return codes from the real sdk
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (OutPresence == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    *OutPresence = nullptr;
    if(Options == nullptr)
        return EOS_EResult::EOS_UnexpectedError;

    auto presence = get_presence(Options->TargetUserId);
    if (presence == nullptr)
        return EOS_EResult::EOS_NotFound;

    EOS_Presence_Info* presence_info = new EOS_Presence_Info;

    presence_info->ApiVersion = EOS_PRESENCE_COPYPRESENCE_API_LATEST;
    char *str;
    size_t len;

    len = presence->platform().length() + 1;
    str = new char[len];
    strncpy(str, presence->platform().c_str(), len);
    presence_info->Platform = str;

    len = presence->productid().length() + 1;
    str = new char[len];
    strncpy(str, presence->productid().c_str(), len);
    presence_info->ProductId = str;

    len = presence->productname().length() + 1;
    str = new char[len];
    strncpy(str, presence->productname().c_str(), len);
    presence_info->ProductName = str;

    len = presence->productversion().length() + 1;
    str = new char[len];
    strncpy(str, presence->productversion().c_str(), len);
    presence_info->ProductVersion = str;

    len = presence->richtext().length() + 1;
    str = new char[len];
    strncpy(str, presence->richtext().c_str(), len);
    presence_info->RichText = str;

    presence_info->RecordsCount = presence->records_size();
    EOS_Presence_DataRecord* records = nullptr;

    if (presence->records_size())
    {
        records = new EOS_Presence_DataRecord[presence->records_size()];
        auto record_it = presence->records().begin();
        for (int i = 0; i < presence->records_size(); ++i)
        {
            len = record_it->first.length() + 1;
            str = new char[len];
            strncpy(str, record_it->first.c_str(), len);
            records[i].Key = str;

            len = record_it->second.length() + 1;
            str = new char[len];
            strncpy(str, record_it->second.c_str(), len);
            records[i].Value = str;

            ++record_it;
        }
    }

    presence_info->Records = records;
    presence_info->Status = (EOS_Presence_EStatus)presence->status();
    presence_info->UserId = Options->TargetUserId;

    *OutPresence = presence_info;
    return EOS_EResult::EOS_Success;
}

/**
 * Creates a presence modification handle. This handle can used to add multiple changes to your presence that can be applied with EOS_Presence_SetPresence.
 * The resulting handle must be released by calling EOS_PresenceModification_Release once it has been passed to EOS_Presence_SetPresence.
 *
 * @param Options Object containing properties related to the user modifying their presence
 * @param OutPresenceModificationHandle Pointer to a Presence Modification Handle to be set if successful
 * @return Success if we successfully created the Presence Modification Handle pointed at in OutPresenceModificationHandle, or an error result if the input data was invalid
 *
 * @see EOS_PresenceModification_Release
 * @see EOS_Presence_SetPresence
 * @see EOS_PresenceModification_SetStatus
 * @see EOS_PresenceModification_SetRawRichText
 * @see EOS_PresenceModification_SetData
 * @see EOS_PresenceModification_DeleteData
 */
EOS_EResult EOSSDK_Presence::CreatePresenceModification( const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle)
{
    // TODO: Check the return codes from the real sdk
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (*Options->LocalUserId == *Settings::Inst().userid)
    {
        EOSSDK_PresenceModification *modification = new EOSSDK_PresenceModification;
        modification->infos = get_myself();
        *OutPresenceModificationHandle = reinterpret_cast<EOS_HPresenceModification>(modification);
        return EOS_EResult::EOS_Success;
    }

    *OutPresenceModificationHandle = nullptr;
    return EOS_EResult::EOS_UnexpectedError;
}

/**
 * Sets your new presence with the data applied to a PresenceModificationHandle. The PresenceModificationHandle can be released safely after calling this function.
 *
 * @param Options Object containing a PresenceModificationHandle and associated user data
 * @param ClientData Optional pointer to help track this request, that is returned in the completion callback
 * @param CompletionDelegate Pointer to a function that handles receiving the completion information
 *
 * @see EOS_Presence_CreatePresenceModification
 * @see EOS_PresenceModification_Release
 */
void EOSSDK_Presence::SetPresence( const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);
    EOS_Presence_SetPresenceCallbackInfo& spci = res->CreateCallback<EOS_Presence_SetPresenceCallbackInfo>((CallbackFunc)CompletionDelegate);
    spci.ClientData = ClientData;
    spci.LocalUserId = Settings::Inst().userid;
    res->done = true;

    if (Options->PresenceModificationHandle == nullptr)
    {
        spci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else if (Options->LocalUserId != Settings::Inst().userid)
    {
        spci.ResultCode = EOS_EResult::EOS_MissingPermissions;
    }
    else
    {
        spci.ResultCode = EOS_EResult::EOS_Success;

        EOSSDK_PresenceModification *new_presence = reinterpret_cast<EOSSDK_PresenceModification*>(Options->PresenceModificationHandle);
        get_myself() = new_presence->infos;

        send_my_presence_info_to_all_peers();
    }

    GetCB_Manager().add_callback(this, res);
}

/**
 * Register to receive notifications when presence changes. If the returned NotificationId is valid, you must call RemoveNotifyOnPresenceChanged when you no longer wish to
 * have your NotificationHandler called
 *
 * @param ClientData Data the is returned to when NotificationHandler is invoked
 * @param NotificationHandler The callback to be fired when a presence change occurs
 * @return Notification ID representing the registered callback if successful, an invalid NotificationId if not
 *
 * @see EOS_ENotificationId::EOS_NotificationId_Invalid
 * @see EOS_Presence_RemoveNotifyOnPresenceChanged
 */
EOS_NotificationId EOSSDK_Presence::AddNotifyOnPresenceChanged( const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    pFrameResult_t res(new FrameResult);
    EOS_Presence_PresenceChangedCallbackInfo& pcci = res->CreateCallback<EOS_Presence_PresenceChangedCallbackInfo>((CallbackFunc)NotificationHandler);
    pcci.ClientData = ClientData;
    pcci.LocalUserId = Settings::Inst().userid;
    pcci.PresenceUserId = GetEpicUserId(sdk::NULL_USER_ID);

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister a previously bound notification handler from receiving presence update notifications
 *
 * @param NotificationId The Notification ID representing the registered callback
 */
void EOSSDK_Presence::RemoveNotifyOnPresenceChanged( EOS_NotificationId NotificationId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

   
    GetCB_Manager().remove_notification(this, NotificationId);
}

/**
 * Register to receive notifications when a user accepts a join game option via the social overlay.
 * @note must call RemoveNotifyJoinGameAccepted to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param Notification A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Presence::AddNotifyJoinGameAccepted( const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn)
{
     LOG(Log::LogLevel::TRACE, "");
     GLOBAL_LOCK();

     pFrameResult_t res(new FrameResult);
     EOS_Presence_JoinGameAcceptedCallbackInfo& jgaci = res->CreateCallback<EOS_Presence_JoinGameAcceptedCallbackInfo>((CallbackFunc)NotificationFn);
     jgaci.ClientData = ClientData;
     jgaci.LocalUserId = Settings::Inst().userid;
     jgaci.TargetUserId = GetEpicUserId(sdk::NULL_USER_ID);
     jgaci.JoinInfo = new char[EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH + 1];

     return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving notifications when a user accepts a join game option via the social overlay.
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Presence::RemoveNotifyJoinGameAccepted( EOS_NotificationId InId)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();
    
    GetCB_Manager().remove_notification(this, InId);
}

/**
 * Gets a join info custom game-data string for a specific user. This is a helper function for reading the presence data related to how a user can be joined.
 * Its meaning is entirely application dependent.
 *
 * This value will be valid only after a QueryPresence call has successfully completed.
 *
 * @param Options Object containing an associated user
 * @param OutBuffer The buffer into which the character data should be written.  The buffer must be long enough to hold a string of EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH.
 * @param InOutBufferLength Used as an input to define the OutBuffer length.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the location string was copied into the OutBuffer.
 *         EOS_Success if the information is available and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if there is user or the location string was not found.
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the location string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 *
 * @see EOS_PRESENCE_LOCATIONSTRING_MAX_LEN
 */
EOS_EResult EOSSDK_Presence::GetJoinInfo( const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    // TODO: Check the return codes from the real sdk
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (Options->TargetUserId == nullptr || InOutBufferLength == nullptr || OutBuffer == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto presence = get_presence(Options->TargetUserId);
    if(presence == nullptr)
        return EOS_EResult::EOS_NotFound;

    size_t len = presence->joininfo().length() + 1;
    if (*InOutBufferLength < len)
    {
        *InOutBufferLength = len;
        return EOS_EResult::EOS_LimitExceeded;
    }

    strncpy(OutBuffer, presence->joininfo().c_str(), len);

    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                           Network Send messages                           //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Presence::send_presence_info_request(Network::peer_t const& peerid, Presence_Info_Request_pb* req)
{
    Network_Message_pb msg;
    Presence_Message_pb* presence = new Presence_Message_pb;

    std::string const& userid = GetEOS_Connect().product_id()->to_string();

    presence->set_allocated_presence_info_request(req);
    msg.set_allocated_presence(presence);

    msg.set_source_id(userid);
    msg.set_dest_id(peerid);

    return GetNetwork().SendTo(msg);
}

bool EOSSDK_Presence::send_my_presence_info(Network::peer_t const& peerid)
{
    Network_Message_pb msg;
    Presence_Message_pb* presence = new Presence_Message_pb;

    std::string const& userid = GetEOS_Connect().product_id()->to_string();

    presence->set_allocated_presence_info(&get_myself());
    msg.set_allocated_presence(presence);

    msg.set_source_id(userid);
    msg.set_dest_id(peerid);

    auto res = GetNetwork().SendTo(msg);
    presence->release_presence_info();

    return res;
}

bool EOSSDK_Presence::send_my_presence_info_to_all_peers()
{
    Network_Message_pb msg;
    Presence_Message_pb* presence = new Presence_Message_pb;

    std::string const& userid = GetEOS_Connect().product_id()->to_string();

    presence->set_allocated_presence_info(&get_myself());
    msg.set_allocated_presence(presence);

    msg.set_source_id(userid);

    GetNetwork().SendToAllPeers(msg).size();
    presence->release_presence_info();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                          Network Receive messages                         //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Presence::on_presence_request(Network_Message_pb const& msg, Presence_Info_Request_pb const& req)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    return send_my_presence_info(msg.source_id());
}

bool EOSSDK_Presence::on_presence_infos(Network_Message_pb const& msg, Presence_Info_pb const& infos)
{
    LOG(Log::LogLevel::TRACE, "");
    GLOBAL_LOCK();

    if (!msg.source_id().empty())
    {
        bool presence_changed = false;
        auto userid = GetEpicUserId(infos.userid());
        Presence_Info_pb& presence_infos = _presences[userid];

        if(presence_infos.status()         != infos.status()         ||
           presence_infos.productid()      != infos.productid()      ||
           presence_infos.productversion() != infos.productversion() ||
           presence_infos.platform()       != infos.platform()       ||
           presence_infos.richtext()       != infos.richtext()       ||
           presence_infos.productname()    != infos.productname()    ||
           presence_infos.joininfo()       != infos.joininfo()       ||
           presence_infos.records_size()   != infos.records_size())
        {
            presence_changed = true;
        }
        else if(presence_infos.records_size())
        {
            for (auto const& record : presence_infos.records())
            {
                auto it = infos.records().find(record.first);
                if (it != infos.records().end())
                {
                    if (record.second != it->second)
                    {
                        presence_changed = true;
                        break;
                    }
                }
                else
                {
                    presence_changed = true;
                    break;
                }
            }
        }

        auto it = _presence_queries.find(userid);
        if (it != _presence_queries.end() && !it->second.empty())
        {
            auto presence_query_it = it->second.begin();

            (*presence_query_it)->done = true;
            (*presence_query_it)->GetCallback<EOS_Presence_QueryPresenceCallbackInfo>().ResultCode = EOS_EResult::EOS_Success;

            it->second.erase(presence_query_it);
        }

        if (presence_changed)
        {
            presence_infos = infos;
            trigger_presence_change(userid);
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Presence::CBRunFrame()
{
    GLOBAL_LOCK();
    return true;
}

bool EOSSDK_Presence::RunNetwork(Network_Message_pb const& msg)
{
    if (msg.source_id() == Settings::Inst().userid->to_string())
        return true;

    Presence_Message_pb const& pres = msg.presence();

    switch (pres.message_case())
    {
        case Presence_Message_pb::MessageCase::kPresenceInfoRequest: return on_presence_request(msg, pres.presence_info_request());
        case Presence_Message_pb::MessageCase::kPresenceInfo       : return on_presence_infos(msg, pres.presence_info());
    }

    return false;
}

bool EOSSDK_Presence::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        case EOS_Presence_QueryPresenceCallbackInfo::k_iCallback:
        {
            EOS_Presence_QueryPresenceCallbackInfo& qpci = res->GetCallback<EOS_Presence_QueryPresenceCallbackInfo>();

            if ((std::chrono::steady_clock::now() - res->created_time) > presence_query_timeout)
            {
                res->done = true;
                qpci.ResultCode = EOS_EResult::EOS_TimedOut;
                
                set_user_status(qpci.TargetUserId, EOS_Presence_EStatus::EOS_PS_Offline);
                auto it = _presence_queries.find(qpci.TargetUserId);
                if (it != _presence_queries.end())
                    _presence_queries.erase(it);
            }
        }
        break;
    }

    return res->done;
}

void EOSSDK_Presence::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case EOS_Presence_QueryPresenceCallbackInfo::k_iCallback:
        //{
        //    EOS_Presence_QueryPresenceCallbackInfo& qpci = res->GetCallback<EOS_Presence_QueryPresenceCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Presence_SetPresenceCallbackInfo::k_iCallback:
        //{// Nothing to free right now
        //    //EOS_Presence_SetPresenceCallbackInfo& spci = res->GetCallback<EOS_Presence_SetPresenceCallbackInfo>();
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case EOS_Presence_PresenceChangedCallbackInfo::k_iCallback:
        //{
        //    EOS_Presence_PresenceChangedCallbackInfo& pcci = res->GetCallback<EOS_Presence_PresenceChangedCallbackInfo>();
        //}
        //break;
        //
        //case EOS_Presence_JoinGameAcceptedCallbackInfo::k_iCallback:
        //{
        //    EOS_Presence_JoinGameAcceptedCallbackInfo& jgaci = res->GetCallback<EOS_Presence_JoinGameAcceptedCallbackInfo>();
        //    delete[]jgaci.JoinInfo;
        //}
    }
}

///////////////////////////////////////////////////////////////////////////////
//                        EOSSDK_PresenceModification                        //
///////////////////////////////////////////////////////////////////////////////
/**
 * Modifies a user's online status to be the new state.
 *
 * @param Options Object containing properties related to setting a user's Status
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 */
EOS_EResult EOSSDK_PresenceModification::SetStatus(const EOS_PresenceModification_SetStatusOptions* Options)
{
    // TODO: Check the return codes from the real sdk
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    if (Options != nullptr)
    {
        infos.set_status((int32_t)Options->Status);
        return EOS_EResult::EOS_Success;
    }
    
    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Modifies a user's Rich Presence string to a new state. This is the exact value other users will see
 * when they query the local user's presence.
 *
 * @param Options Object containing properties related to setting a user's RichText string
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_RICH_TEXT_MAX_VALUE_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::SetRawRichText(const EOS_PresenceModification_SetRawRichTextOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    if (Options != nullptr && Options->RichText != nullptr)
    {
        infos.set_richtext(Options->RichText);
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Modifies one or more rows of user-defined presence data for a local user. At least one InfoData object
 * must be specified.
 *
 * @param Options Object containing an array of new presence data.
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_DATA_MAX_KEYS
 * @see EOS_PRESENCE_DATA_MAX_KEY_LENGTH
 * @see EOS_PRESENCE_DATA_MAX_VALUE_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::SetData(const EOS_PresenceModification_SetDataOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    if (Options != nullptr)
    {
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            if (strlen(Options->Records[i].Key) > EOS_PRESENCE_DATA_MAX_KEY_LENGTH)
                return EOS_EResult::EOS_Presence_DataKeyLengthInvalid;

            if (strlen(Options->Records[i].Value) > EOS_PRESENCE_DATA_MAX_VALUE_LENGTH)
                return EOS_EResult::EOS_Presence_DataValueLengthInvalid;
        }

        auto &records = *infos.mutable_records();
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            records[Options->Records[i].Key] = Options->Records[i].Value;
        }
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Removes one or more rows of user-defined presence data for a local user. At least one DeleteDataInfo object
 * must be specified.
 *
 * @param Options Object containing an array of new presence data.
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_DATA_MAX_KEYS
 * @see EOS_PRESENCE_DATA_MAX_KEY_LENGTH
 * @see EOS_PRESENCE_DATA_MAX_VALUE_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::DeleteData(const EOS_PresenceModification_DeleteDataOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    if (Options != nullptr)
    {
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            if (strlen(Options->Records[i].Key) > EOS_PRESENCE_DATA_MAX_KEY_LENGTH)
                return EOS_EResult::EOS_Presence_DataKeyLengthInvalid;
        }

        auto& records = *infos.mutable_records();
        for (int i = 0; i < Options->RecordsCount; ++i)
        {
            auto it = records.find(Options->Records[i].Key);
            if (it == records.end())
                records.erase(it);
        }
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

/**
 * Sets your new join info custom game-data string. This is a helper function for reading the presence data related to how a user can be joined.
 * Its meaning is entirely application dependent.
 *
 * @param Options Object containing a join info string and associated user data
 * @return Success if modification was added successfully, otherwise an error code related to the problem
 *
 * @see EOS_PRESENCE_JOIN_INFO_STRING_LENGTH
 */
EOS_EResult EOSSDK_PresenceModification::SetJoinInfo(const EOS_PresenceModification_SetJoinInfoOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");
    LOCAL_LOCK();

    if (Options != nullptr && Options->JoinInfo != nullptr)
    {
        if (strlen(Options->JoinInfo) > EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH)
            return EOS_EResult::EOS_UnexpectedError;
        
        infos.set_joininfo(Options->JoinInfo);
        return EOS_EResult::EOS_Success;
    }

    return EOS_EResult::EOS_InvalidParameters;
}

}