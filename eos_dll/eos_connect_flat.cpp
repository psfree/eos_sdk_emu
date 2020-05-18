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

using namespace sdk;

/**
 * The Connect Interface is used to manage local user permissions and access to backend services through the verification of various forms of credentials.
 * It creates an association between third party providers and an internal mapping that allows Epic Online Services to represent a user agnostically
 * All Connect Interface calls take a handle of type EOS_HConnect as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetConnectInterface function.
 *
 * NOTE: At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetConnectInterface
 */

EOS_DECLARE_FUNC(void) EOS_Connect_Login(EOS_HConnect Handle, const EOS_Connect_LoginOptions* Options, void* ClientData, const EOS_Connect_OnLoginCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->Login(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateUser(EOS_HConnect Handle, const EOS_Connect_CreateUserOptions* Options, void* ClientData, const EOS_Connect_OnCreateUserCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->CreateUser(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_LinkAccount(EOS_HConnect Handle, const EOS_Connect_LinkAccountOptions* Options, void* ClientData, const EOS_Connect_OnLinkAccountCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->LinkAccount(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateDeviceId(EOS_HConnect Handle, const EOS_Connect_CreateDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnCreateDeviceIdCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->CreateDeviceId(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_DeleteDeviceId(EOS_HConnect Handle, const EOS_Connect_DeleteDeviceIdOptions* Options, void* ClientData, const EOS_Connect_OnDeleteDeviceIdCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->DeleteDeviceId(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryExternalAccountMappings(EOS_HConnect Handle, const EOS_Connect_QueryExternalAccountMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryExternalAccountMappingsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->QueryExternalAccountMappings(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryProductUserIdMappings(EOS_HConnect Handle, const EOS_Connect_QueryProductUserIdMappingsOptions* Options, void* ClientData, const EOS_Connect_OnQueryProductUserIdMappingsCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->QueryProductUserIdMappings(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetExternalAccountMapping(EOS_HConnect Handle, const EOS_Connect_GetExternalAccountMappingsOptions* Options)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->GetExternalAccountMapping(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_GetProductUserIdMapping(EOS_HConnect Handle, const EOS_Connect_GetProductUserIdMappingOptions* Options, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->GetProductUserIdMapping(Options, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(int32_t) EOS_Connect_GetLoggedInUsersCount(EOS_HConnect Handle)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->GetLoggedInUsersCount();
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetLoggedInUserByIndex(EOS_HConnect Handle, int32_t Index)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->GetLoggedInUserByIndex(Index);
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Connect_GetLoginStatus(EOS_HConnect Handle, EOS_ProductUserId LocalUserId)
{
    if (Handle == nullptr)
        return EOS_ELoginStatus::EOS_LS_NotLoggedIn;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->GetLoginStatus(LocalUserId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyAuthExpiration(EOS_HConnect Handle, const EOS_Connect_AddNotifyAuthExpirationOptions* Options, void* ClientData, const EOS_Connect_OnAuthExpirationCallback Notification)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->AddNotifyAuthExpiration(Options, ClientData, Notification);
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyAuthExpiration(EOS_HConnect Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->RemoveNotifyAuthExpiration(InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyLoginStatusChanged(EOS_HConnect Handle, const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Connect_OnLoginStatusChangedCallback Notification)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->AddNotifyLoginStatusChanged(Options, ClientData, Notification);
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyLoginStatusChanged(EOS_HConnect Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    pInst->RemoveNotifyLoginStatusChanged(InId);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Connect_GetProductUserExternalAccountCount(EOS_HConnect Handle, const EOS_Connect_GetProductUserExternalAccountCountOptions * Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->GetProductUserExternalAccountCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserExternalAccountByIndex(EOS_HConnect Handle, const EOS_Connect_CopyProductUserExternalAccountByIndexOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->CopyProductUserExternalAccountByIndex(Options, OutExternalAccountInfo);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserExternalAccountByAccountType(EOS_HConnect Handle, const EOS_Connect_CopyProductUserExternalAccountByAccountTypeOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->CopyProductUserExternalAccountByAccountType(Options, OutExternalAccountInfo);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserExternalAccountByAccountId(EOS_HConnect Handle, const EOS_Connect_CopyProductUserExternalAccountByAccountIdOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->CopyProductUserExternalAccountByAccountId(Options, OutExternalAccountInfo);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserInfo(EOS_HConnect Handle, const EOS_Connect_CopyProductUserInfoOptions* Options, EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Connect*>(Handle);
    return pInst->CopyProductUserInfo(Options, OutExternalAccountInfo);
}

EOS_DECLARE_FUNC(void) EOS_Connect_ExternalAccountInfo_Release(EOS_Connect_ExternalAccountInfo* ExternalAccountInfo)
{
    if (ExternalAccountInfo != nullptr)
    {
        delete ExternalAccountInfo;
    }
}