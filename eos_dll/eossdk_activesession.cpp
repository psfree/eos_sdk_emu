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

/**
 * Representation of an existing session some local players are actively involved in (via Create/Join)
 */

 /**
  * EOS_ActiveSession_CopyInfo is used to immediately retrieve a copy of active session information
  * If the call returns an EOS_Success result, the out parameter, OutActiveSessionInfo, must be passed to EOS_ActiveSession_Info_Release to release the memory associated with it.
  *
  * @param Options Structure containing the input parameters
  * @param OutActiveSessionInfo Out parameter used to receive the EOS_ActiveSession_Info structure.
  *
  * @return EOS_Success if the information is available and passed out in OutActiveSessionInfo
  *         EOS_InvalidParameters if you pass a null pointer for the out parameter
  *         EOS_IncompatibleVersion if the API version passed in is incorrect
  *
  * @see EOS_ActiveSession_Info
  * @see EOS_ActiveSession_CopyInfoOptions
  * @see EOS_ActiveSession_Info_Release
  */
EOS_EResult EOSSDK_ActiveSession::CopyInfo(const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info** OutActiveSessionInfo)
{
    TRACE_FUNC();

    EOS_ActiveSession_Info* copy_session_info = new EOS_ActiveSession_Info;
    EOS_SessionDetails_Info* session_details_info = new EOS_SessionDetails_Info;
    EOS_SessionDetails_Settings* session_details_settings = new EOS_SessionDetails_Settings;

    // ActiveSession_Info
    copy_session_info->ApiVersion = EOS_ACTIVESESSION_INFO_API_LATEST;
    copy_session_info->LocalUserId = Settings::Inst().productuserid;
    copy_session_info->State = static_cast<EOS_EOnlineSessionState>(_infos.state());
    {
        size_t len = _session_name.length() + 1;
        char* str = new char[len];
        strncpy(str, _session_name.c_str(), len);
        copy_session_info->SessionName = str;
    }
    copy_session_info->SessionDetails = session_details_info;

    // SessionDetails_Info
    session_details_info->ApiVersion = EOS_SESSIONDETAILS_INFO_API_LATEST;
    session_details_info->NumOpenPublicConnections = _infos.max_players() - _infos.players_size();
    {
        size_t len = _infos.session_id().length() + 1;
        char* str = new char[len];
        strncpy(str, _infos.session_id().c_str(), len);
        session_details_info->SessionId = str;
    }
    {
        size_t len = _infos.host_address().length() + 1;
        char* str = new char[len];
        strncpy(str, _infos.host_address().c_str(), len);
        session_details_info->HostAddress = str;
    }
    session_details_info->Settings = session_details_settings;

    // SessionDetails_Settings
    session_details_settings->ApiVersion = EOS_SESSIONDETAILS_SETTINGS_API_LATEST;
    session_details_settings->bAllowJoinInProgress = _infos.join_in_progress_allowed();
    session_details_settings->bInvitesAllowed = _infos.invites_allowed();
    {
        size_t len = _infos.bucket_id().length() + 1;
        char* str = new char[len];
        strncpy(str, _infos.bucket_id().c_str(), len);
        session_details_settings->BucketId = str;
    }
    session_details_settings->NumPublicConnections = _infos.max_players();
    session_details_settings->PermissionLevel = static_cast<EOS_EOnlineSessionPermissionLevel>(_infos.permission_level());

    *OutActiveSessionInfo = copy_session_info;

    return EOS_EResult::EOS_Success;
}

/**
 * Get the number of registered players associated with this active session
 *
 * @param Options the Options associated with retrieving the registered player count
 *
 * @return number of registered players in the active session or 0 if there is an error
 */
uint32_t EOSSDK_ActiveSession::GetRegisteredPlayerCount(const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options)
{
    TRACE_FUNC();

    return _infos.registered_players_size();
}

/**
 * EOS_ActiveSession_GetRegisteredPlayerByIndex is used to immediately retrieve individual players registered with the active session.
 *
 * @param Options Structure containing the input parameters
 *
 * @return the product user id for the registered player at a given index or null if that index is invalid
 *
 * @see EOS_ActiveSession_GetRegisteredPlayerCount
 * @see EOS_ActiveSession_GetRegisteredPlayerByIndexOptions
 */
EOS_ProductUserId EOSSDK_ActiveSession::GetRegisteredPlayerByIndex(const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options)
{
    TRACE_FUNC();

    if (Options->PlayerIndex >= static_cast<uint32_t>(_infos.registered_players_size()))
        return GetInvalidProductUserId();

    return GetProductUserId(_infos.registered_players()[Options->PlayerIndex]);
}

/**
 * Release the memory associated with an active session.
 * This must be called on data retrieved from EOS_Sessions_CopyActiveSessionHandle
 *
 * @param ActiveSessionHandle - The active session handle to release
 *
 * @see EOS_Sessions_CopyActiveSessionHandle
 */
void EOSSDK_ActiveSession::Release()
{
    TRACE_FUNC();

    delete this;
}

}