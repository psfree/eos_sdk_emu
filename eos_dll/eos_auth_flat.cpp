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

#include "eossdk_auth.h"

using namespace sdk;

EOS_DECLARE_FUNC(void) EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->Login(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_Logout(EOS_HAuth Handle, const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->Logout(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_LinkAccount(EOS_HAuth Handle, const EOS_Auth_LinkAccountOptions* Options, void* ClientData, const EOS_Auth_OnLinkAccountCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->LinkAccount(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_DeletePersistentAuth(EOS_HAuth Handle, const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->DeletePersistentAuth(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(void) EOS_Auth_VerifyUserAuth(EOS_HAuth Handle, const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->VerifyUserAuth(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(int32_t) EOS_Auth_GetLoggedInAccountsCount(EOS_HAuth Handle)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->GetLoggedInAccountsCount();
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetLoggedInAccountByIndex(EOS_HAuth Handle, int32_t Index)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->GetLoggedInAccountByIndex(Index);
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
{
    if (Handle == nullptr)
        return EOS_ELoginStatus::EOS_LS_NotLoggedIn;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->GetLoginStatus(LocalUserId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenOld(EOS_HAuth Handle, EOS_AccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->CopyUserAuthTokenOld(LocalUserId, OutUserAuthToken);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthTokenNew(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->CopyUserAuthToken(Options, LocalUserId, OutUserAuthToken);
}

#ifdef _MSC_VER
#pragma optimize("", off)
#endif

#if defined(__WINDOWS_32__)
#pragma comment(linker, "/export:_EOS_Auth_CopyUserAuthToken@16=_EOS_Auth_CopyUserAuthTokenNew@16")
#pragma comment(linker, "/export:_EOS_Auth_CopyUserAuthToken@12=_EOS_Auth_CopyUserAuthTokenOld@12")
#else
EOS_DECLARE_FUNC(EOS_EResult) CLANG_GCC_DONT_OPTIMIZE EOS_Auth_CopyUserAuthToken()
{
    // Build rewrittable opcodes, need 14 for x64 absolute jmp and 5 for x86 relative jmp
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    EOS_Auth_CopyUserAuthTokenOld(nullptr, nullptr, nullptr);
    return EOS_EResult::EOS_NotImplemented;
}
#endif
#ifdef _MSC_VER
#pragma optimize("", on)
#endif

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedOld(EOS_HAuth Handle, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->AddNotifyLoginStatusChangedOld(ClientData, Notification);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChangedNew(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    return pInst->AddNotifyLoginStatusChanged(Options, ClientData, Notification);
}

#ifdef _MSC_VER
#pragma optimize("", off)
#endif
#if defined(__WINDOWS_32__)
#pragma comment(linker, "/export:_EOS_Auth_AddNotifyLoginStatusChanged@16=_EOS_Auth_AddNotifyLoginStatusChangedNew@16")
#pragma comment(linker, "/export:_EOS_Auth_AddNotifyLoginStatusChanged@12=_EOS_Auth_AddNotifyLoginStatusChangedOld@12")
#else
EOS_DECLARE_FUNC(EOS_NotificationId) CLANG_GCC_DONT_OPTIMIZE EOS_Auth_AddNotifyLoginStatusChanged()
{
    // Build rewrittable opcodes, need 14 for x64 absolute jmp and 5 for x86 relative jmp
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    EOS_Auth_AddNotifyLoginStatusChangedOld(nullptr, nullptr, nullptr);
    return EOS_INVALID_NOTIFICATIONID;
}
#endif
#ifdef _MSC_VER
#pragma optimize("", on)
#endif

EOS_DECLARE_FUNC(void) EOS_Auth_RemoveNotifyLoginStatusChanged(EOS_HAuth Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Auth*>(Handle);
    pInst->RemoveNotifyLoginStatusChanged(InId);
}


EOS_DECLARE_FUNC(void) EOS_Auth_Token_Release(EOS_Auth_Token* AuthToken)
{
    TRACE_FUNC();
    if (AuthToken == nullptr)
        return;

    delete[] AuthToken->AccessToken;
    delete[] AuthToken->RefreshToken;

    delete[] AuthToken->ExpiresAt;
    delete[] AuthToken->RefreshExpiresAt;
    delete AuthToken;
}

