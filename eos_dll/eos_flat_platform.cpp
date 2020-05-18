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

#include "eossdk_platform.h"

using namespace sdk;

/////////////////////////////////////////////////////////
// SDK

EOS_DECLARE_FUNC(void) EOS_Platform_Tick(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    pInst->Tick();
}

EOS_DECLARE_FUNC(EOS_HMetrics) EOS_Platform_GetMetricsInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetMetricsInterface();
}

EOS_DECLARE_FUNC(EOS_HAuth) EOS_Platform_GetAuthInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetAuthInterface();
}

EOS_DECLARE_FUNC(EOS_HConnect) EOS_Platform_GetConnectInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetConnectInterface();
}

EOS_DECLARE_FUNC(EOS_HEcom) EOS_Platform_GetEcomInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetEcomInterface();
}

EOS_DECLARE_FUNC(EOS_HUI) EOS_Platform_GetUIInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetUIInterface();
}

EOS_DECLARE_FUNC(EOS_HFriends) EOS_Platform_GetFriendsInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetFriendsInterface();
}

EOS_DECLARE_FUNC(EOS_HPresence) EOS_Platform_GetPresenceInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetPresenceInterface();
}
EOS_DECLARE_FUNC(EOS_HSessions) EOS_Platform_GetSessionsInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetSessionsInterface();
}

EOS_DECLARE_FUNC(EOS_HLobby) EOS_Platform_GetLobbyInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetLobbyInterface();
}

EOS_DECLARE_FUNC(EOS_HUserInfo) EOS_Platform_GetUserInfoInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetUserInfoInterface();
}

EOS_DECLARE_FUNC(EOS_HP2P) EOS_Platform_GetP2PInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetP2PInterface();
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorage) EOS_Platform_GetPlayerDataStorageInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetPlayerDataStorageInterface();
}

EOS_DECLARE_FUNC(EOS_HAchievements) EOS_Platform_GetAchievementsInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetAchievementsInterface();
}

EOS_DECLARE_FUNC(EOS_HStats) EOS_Platform_GetStatsInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetStatsInterface();
}

EOS_DECLARE_FUNC(EOS_HLeaderboards) EOS_Platform_GetLeaderboardsInterface(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return nullptr;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetLeaderboardsInterface();
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveCountryCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetActiveCountryCode(LocalUserId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveLocaleCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetActiveLocaleCode(LocalUserId, OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideCountryCode(EOS_HPlatform Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetOverrideCountryCode(OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideLocaleCode(EOS_HPlatform Handle, char* OutBuffer, int32_t* InOutBufferLength)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->GetOverrideLocaleCode(OutBuffer, InOutBufferLength);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideCountryCode(EOS_HPlatform Handle, const char* NewCountryCode)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->SetOverrideCountryCode(NewCountryCode);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideLocaleCode(EOS_HPlatform Handle, const char* NewLocaleCode)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->SetOverrideLocaleCode(NewLocaleCode);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_CheckForLauncherAndRestart(EOS_HPlatform Handle)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);
    return pInst->CheckForLauncherAndRestart();
}

/////////////////////////////////////////////////////////
// platform

/**
 * Create a single Epic Online Services Platform Instance.
 *
 * The platform instance is used to gain access to the various Epic Online Services.
 *
 * This function returns an opaque handle to the platform instance, and that handle must be passed to EOS_Platform_Release to release the instance.
 *
 * @return An opaque handle to the platform instance.
 */
EOS_DECLARE_FUNC(EOS_HPlatform) EOS_Platform_Create(const EOS_Platform_Options* Options)
{
    TRACE_FUNC();

    auto &inst = EOSSDK_Platform::Inst();
    inst.Init(Options);
    return reinterpret_cast<EOS_HPlatform>(&inst);
}

/**
 * Release an Epic Online Services platform instance previously returned from EOS_Platform_Create.
 *
 * This function should only be called once per instance returned by EOS_Platform_Create. Undefined behavior will result in calling it with a single instance more than once.
 * Typically only a single platform instance needs to be created during the lifetime of a game.
 * You should release each platform instance before calling the EOS_Shutdown function.
 */
EOS_DECLARE_FUNC(void) EOS_Platform_Release(EOS_HPlatform Handle)
{
    auto pInst = reinterpret_cast<EOSSDK_Platform*>(Handle);

    if (pInst != &EOSSDK_Platform::Inst())
        return;

    pInst->Release();
}

