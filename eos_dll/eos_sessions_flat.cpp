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
#include "eos_client_api.h"

using namespace sdk;

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionModification(EOS_HSessions Handle, const EOS_Sessions_CreateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->CreateSessionModification(Options, OutSessionModificationHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_UpdateSessionModification(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->UpdateSessionModification(Options, OutSessionModificationHandle);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UpdateSession(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionOptions* Options, void* ClientData, const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->UpdateSession(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_DestroySession(EOS_HSessions Handle, const EOS_Sessions_DestroySessionOptions* Options, void* ClientData, const EOS_Sessions_OnDestroySessionCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->DestroySession(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_JoinSession(EOS_HSessions Handle, const EOS_Sessions_JoinSessionOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->JoinSession(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_StartSession(EOS_HSessions Handle, const EOS_Sessions_StartSessionOptions* Options, void* ClientData, const EOS_Sessions_OnStartSessionCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->StartSession(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_EndSession(EOS_HSessions Handle, const EOS_Sessions_EndSessionOptions* Options, void* ClientData, const EOS_Sessions_OnEndSessionCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->EndSession(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RegisterPlayers(EOS_HSessions Handle, const EOS_Sessions_RegisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->RegisterPlayers(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UnregisterPlayers(EOS_HSessions Handle, const EOS_Sessions_UnregisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->UnregisterPlayers(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_SendInvite(EOS_HSessions Handle, const EOS_Sessions_SendInviteOptions* Options, void* ClientData, const EOS_Sessions_OnSendInviteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->SendInvite(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RejectInvite(EOS_HSessions Handle, const EOS_Sessions_RejectInviteOptions* Options, void* ClientData, const EOS_Sessions_OnRejectInviteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->RejectInvite(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_QueryInvites(EOS_HSessions Handle, const EOS_Sessions_QueryInvitesOptions* Options, void* ClientData, const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->QueryInvites(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Sessions_GetInviteCount(EOS_HSessions Handle, const EOS_Sessions_GetInviteCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->GetInviteCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_GetInviteIdByIndex(EOS_HSessions Handle, const EOS_Sessions_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->GetInviteIdByIndex(Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionSearch(EOS_HSessions Handle, const EOS_Sessions_CreateSessionSearchOptions* Options, EOS_HSessionSearch* OutSessionSearchHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->CreateSessionSearch(Options, OutSessionSearchHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopyActiveSessionHandle(EOS_HSessions Handle, const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->CopyActiveSessionHandle(Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteReceived(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn)
{    
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->AddNotifySessionInviteReceived(Options, ClientData, NotificationFn);;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteReceived(EOS_HSessions Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->RemoveNotifySessionInviteReceived(InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->AddNotifySessionInviteAccepted(Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteAccepted(EOS_HSessions Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    pInst->RemoveNotifySessionInviteAccepted(InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyJoinSessionAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifyJoinSessionAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionAcceptedCallback NotificationFn)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->AddNotifyJoinSessionAccepted(Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyJoinSessionAccepted(EOS_HSessions Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->RemoveNotifyJoinSessionAccepted(InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByInviteId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->CopySessionHandleByInviteId(Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByUiEventId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByUiEventIdOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->CopySessionHandleByUiEventId(Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleForPresence(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->CopySessionHandleForPresence(Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_IsUserInSession(EOS_HSessions Handle, const EOS_Sessions_IsUserInSessionOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->IsUserInSession(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_DumpSessionState(EOS_HSessions Handle, const EOS_Sessions_DumpSessionStateOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Sessions*>(Handle);
    return pInst->DumpSessionState(Options);
}

/**
 * To modify sessions, you must call EOS_Sessions_CreateSessionModification to create a Session Modification handle. To modify that handle, call
 * EOS_SessionModification_* methods. Once you are finished, call EOS_Sessions_UpdateSession with your handle. You must then release your Session Modification
 * handle by calling EOS_SessionModification_Release.
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetBucketId(EOS_HSessionModification Handle, const EOS_SessionModification_SetBucketIdOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->SetBucketId(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetHostAddress(EOS_HSessionModification Handle, const EOS_SessionModification_SetHostAddressOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->SetHostAddress(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetPermissionLevel(EOS_HSessionModification Handle, const EOS_SessionModification_SetPermissionLevelOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->SetPermissionLevel(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetJoinInProgressAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->SetJoinInProgressAllowed(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetMaxPlayers(EOS_HSessionModification Handle, const EOS_SessionModification_SetMaxPlayersOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->SetMaxPlayers(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetInvitesAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetInvitesAllowedOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->SetInvitesAllowed(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_AddAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_AddAttributeOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->AddAttribute(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_RemoveAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_RemoveAttributeOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(Handle);
    return pInst->RemoveAttribute(Options);
}

/**
 * Representation of an existing session some local players are actively involved in (via Create/Join)
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_ActiveSession_CopyInfo(EOS_HActiveSession Handle, const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info** OutActiveSessionInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_ActiveSession*>(Handle);
    return pInst->CopyInfo(Options, OutActiveSessionInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_ActiveSession_GetRegisteredPlayerCount(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_ActiveSession*>(Handle);
    return pInst->GetRegisteredPlayerCount(Options);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ActiveSession_GetRegisteredPlayerByIndex(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options)
{
    if (Handle == nullptr)
        return GetInvalidProductUserId();

    auto pInst = reinterpret_cast<EOSSDK_ActiveSession*>(Handle);
    return pInst->GetRegisteredPlayerByIndex(Options);
}

/**
 * This class represents the details of a session, including its session properties and the attribution associated with it
 * Locally created or joined active sessions will contain this information as will search results.
 * A handle to a session is required to join a session via search or invite
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopyInfo(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info** OutSessionInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionDetails*>(Handle);
    return pInst->CopyInfo(Options, OutSessionInfo);
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionDetails_GetSessionAttributeCount(EOS_HSessionDetails Handle, const EOS_SessionDetails_GetSessionAttributeCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_SessionDetails*>(Handle);
    return pInst->GetSessionAttributeCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByIndex(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionDetails*>(Handle);
    return pInst->CopySessionAttributeByIndex(Options, OutSessionAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByKey(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionDetails*>(Handle);
    return pInst->CopySessionAttributeByKey(Options, OutSessionAttribute);
}

/**
 * Class responsible for the creation, setup, and execution of a search query.
 * Search parameters are defined, the query is executed and the search results are returned within this object
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetSessionId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetSessionIdOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->SetSessionId(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetTargetUserId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetTargetUserIdOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->SetTargetUserId(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetParameterOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->SetParameter(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_RemoveParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_RemoveParameterOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->RemoveParameter(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetMaxResults(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetMaxResultsOptions* Options)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->SetMaxResults(Options);
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Find(EOS_HSessionSearch Handle, const EOS_SessionSearch_FindOptions* Options, void* ClientData, const EOS_SessionSearch_OnFindCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return ;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    pInst->Find(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionSearch_GetSearchResultCount(EOS_HSessionSearch Handle, const EOS_SessionSearch_GetSearchResultCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->GetSearchResultCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_CopySearchResultByIndex(EOS_HSessionSearch Handle, const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(Handle);
    return pInst->CopySearchResultByIndex(Options, OutSessionHandle);
}

EOS_DECLARE_FUNC(void) EOS_SessionModification_Release(EOS_HSessionModification SessionModificationHandle)
{
    if (SessionModificationHandle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_SessionModification*>(SessionModificationHandle);
    pInst->Release();
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Release(EOS_HActiveSession ActiveSessionHandle)
{
    if (ActiveSessionHandle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_ActiveSession*>(ActiveSessionHandle);
    pInst->Release();
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Release(EOS_HSessionDetails SessionHandle)
{
    if (SessionHandle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_SessionDetails*>(SessionHandle);
    pInst->Release();
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Release(EOS_HSessionSearch SessionSearchHandle)
{
    if (SessionSearchHandle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_SessionSearch*>(SessionSearchHandle);
    pInst->Release();
}

/**
 * Release the memory associated with a session attribute. This must be called on data retrieved from EOS_SessionDetails_CopySessionAttributeByIndex.
 *
 * @param SessionAttribute - The session attribute to release
 *
 * @see EOS_SessionDetails_CopySessionAttributeByIndex
 */
EOS_DECLARE_FUNC(void) EOS_SessionDetails_Attribute_Release(EOS_SessionDetails_Attribute* SessionAttribute)
{
    TRACE_FUNC();

    if (SessionAttribute == nullptr)
        return;

    if(SessionAttribute->Data->ValueType == EOS_ESessionAttributeType::EOS_AT_STRING)
        delete[]SessionAttribute->Data->Value.AsUtf8;

    delete[]SessionAttribute->Data->Key;
    delete SessionAttribute->Data;

    delete SessionAttribute;
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Info_Release(EOS_SessionDetails_Info* SessionInfo)
{
    TRACE_FUNC();

    if (SessionInfo == nullptr)
        return;

    // SessionDetails_Settings
    delete[] SessionInfo->Settings->BucketId;
    delete SessionInfo->Settings;

    // SessionDetails_Info
    delete[] SessionInfo->HostAddress;
    delete[] SessionInfo->SessionId;
    delete SessionInfo;
}

/**
 * Release the memory associated with an EOS_ActiveSession_Info structure. This must be called on data retrieved from EOS_ActiveSession_CopyInfo.
 *
 * @param ActiveSessionInfo - The active session structure to be released
 *
 * @see EOS_ActiveSession_Info
 * @see EOS_ActiveSession_CopyInfo
 */
EOS_DECLARE_FUNC(void) EOS_ActiveSession_Info_Release(EOS_ActiveSession_Info* ActiveSessionInfo)
{
    TRACE_FUNC();

    if (ActiveSessionInfo == nullptr)
        return;

    EOS_SessionDetails_Info_Release(const_cast<EOS_SessionDetails_Info*>(ActiveSessionInfo->SessionDetails));
    
    // ActiveSession_Info
    delete[]ActiveSessionInfo->SessionName;
    delete ActiveSessionInfo;
}