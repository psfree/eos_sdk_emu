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
#include "settings.h"

namespace sdk
{

EOSSDK_Platform::EOSSDK_Platform():
    _platform_init(false),

    _cb_manager       (nullptr),
    _network          (nullptr),
    _metrics          (nullptr),
    _auth             (nullptr),
    _connect          (nullptr),
    _ecom             (nullptr),
    _ui               (nullptr),
    _friends          (nullptr),
    _presence         (nullptr),
    _sessions         (nullptr),
    _lobby            (nullptr),
    _userinfo         (nullptr),
    _p2p              (nullptr),
    _playerdatastorage(nullptr),
    _achievements     (nullptr),
    _stats            (nullptr),
    _leaderboards     (nullptr)
{
    _cb_manager        = new Callback_Manager;
    _network           = new Network;
}

EOSSDK_Platform::~EOSSDK_Platform()
{
    Release();
    delete _network;
    delete _cb_manager;
}

EOSSDK_Platform& EOSSDK_Platform::Inst()
{
    static EOSSDK_Platform instance;
    return instance;
}

void EOSSDK_Platform::Init(const EOS_Platform_Options* options)
{
    GLOBAL_LOCK();
    if(!_platform_init)
    {
        if (options != nullptr)
        {
            _api_version = options->ApiVersion;
            switch (options->ApiVersion)
            {
                case EOS_PLATFORM_OPTIONS_API_006:
                {
                    auto pf = reinterpret_cast<const EOS_Platform_Options006*>(options);
                    if (pf->CacheDirectory != nullptr)
                        _cache_directory = pf->CacheDirectory;

                    LOG(Log::LogLevel::DEBUG, "CacheDirectory = '%s'", _cache_directory.c_str());
                }
                case EOS_PLATFORM_OPTIONS_API_005:
                {
                    auto pf = reinterpret_cast<const EOS_Platform_Options005*>(options);
                    if (pf->EncryptionKey != nullptr)
                        _encryption_key = pf->EncryptionKey;

                    if (pf->OverrideCountryCode != nullptr)
                        _override_country_code = pf->OverrideCountryCode;

                    if (pf->OverrideLocaleCode != nullptr)
                        _override_locale_code = pf->OverrideLocaleCode;

                    if (pf->DeploymentId != nullptr)
                        _deployment_id = pf->DeploymentId;

                    _flags = pf->Flags;

                    LOG(Log::LogLevel::DEBUG, "EncryptionKey = '%s'", _encryption_key.c_str());
                    LOG(Log::LogLevel::DEBUG, "OverrideCountryCode = '%s'", _override_country_code.c_str());
                    LOG(Log::LogLevel::DEBUG, "OverrideLocaleCode = '%s'", _override_locale_code.c_str());
                    LOG(Log::LogLevel::DEBUG, "DeploymentId = '%s'", _deployment_id.c_str());
                    LOG(Log::LogLevel::DEBUG, "Flags = %llu", _flags);
                }
                case EOS_PLATFORM_OPTIONS_API_001:
                {
                    auto pf = reinterpret_cast<const EOS_Platform_Options001*>(options);
                    _reserved = pf->Reserved;

                    if (pf->ProductId != nullptr)
                        _product_id = pf->ProductId;

                    if (pf->SandboxId != nullptr)
                        _sandbox_id = pf->SandboxId;

                    if (pf->ClientCredentials.ClientId != nullptr)
                        _client_id = pf->ClientCredentials.ClientId;

                    if (pf->ClientCredentials.ClientSecret != nullptr)
                        _client_secret = pf->ClientCredentials.ClientSecret;

                    _is_server = pf->bIsServer;

                    LOG(Log::LogLevel::DEBUG, "ProductId = '%s'", _product_id.c_str());
                    LOG(Log::LogLevel::DEBUG, "SandboxId = '%s'", _sandbox_id.c_str());
                    LOG(Log::LogLevel::DEBUG, "ClientId = '%s'", _client_id.c_str());
                    LOG(Log::LogLevel::DEBUG, "ClientSecret = '%s'", _client_secret.c_str());
                    LOG(Log::LogLevel::DEBUG, "ApiVersion = %u", pf->ApiVersion);
                }
            }
        }

        _auth              = new EOSSDK_Auth;
        _friends           = new EOSSDK_Friends;
        _presence          = new EOSSDK_Presence;
        _connect           = new EOSSDK_Connect;
        _metrics           = new EOSSDK_Metrics;
        _ecom              = new EOSSDK_Ecom;
        _ui                = new EOSSDK_UI;
        _sessions          = new EOSSDK_Sessions;
        _lobby             = new EOSSDK_Lobby;
        _userinfo          = new EOSSDK_UserInfo;
        _p2p               = new EOSSDK_P2P;
        _playerdatastorage = new EOSSDK_PlayerDataStorage;
        _achievements      = new EOSSDK_Achievements;
        _stats             = new EOSSDK_Stats;
        _leaderboards      = new EOSSDK_Leaderboards;

        _presence->setup_myself();
        _userinfo->setup_myself();

        _platform_init = true;
    }
}

void EOSSDK_Platform::Release()
{
    GLOBAL_LOCK();

    if (_platform_init)
    {
        delete _leaderboards;
        delete _stats;
        delete _achievements;
        delete _playerdatastorage;
        delete _p2p;
        delete _userinfo;
        delete _lobby;
        delete _sessions;
        delete _presence;
        delete _friends;
        delete _ui;
        delete _ecom;
        delete _connect;
        delete _auth;
        delete _metrics;

        _platform_init = false;
    }
}

/**
 * Notify the platform instance to do work. This function must be called frequently in order for the services provided by the SDK to properly
 * function. For tick-based applications, it is usually desireable to call this once per-tick.
 */
void EOSSDK_Platform::Tick()
{
    GLOBAL_LOCK();
    GetCB_Manager().run_frames();
    GetCB_Manager().run_callbacks();
}

/**
 * Get a handle to the Metrics Interface.
 * @return EOS_HMetrics handle
 *
 * @see eos_metrics.h
 * @see eos_metrics_types.h
 */
EOS_HMetrics           EOSSDK_Platform::GetMetricsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HMetrics>(_metrics);
}

/**
 * Get a handle to the Auth Interface.
 * @return EOS_HAuth handle
 *
 * @see eos_auth.h
 * @see eos_auth_types.h
 */
EOS_HAuth              EOSSDK_Platform::GetAuthInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HAuth>(_auth);
}

/**
 * Get a handle to the Connect Interface.
 * @return EOS_HConnect handle
 *
 * @see eos_connect.h
 * @see eos_connect_types.h
 */
EOS_HConnect           EOSSDK_Platform::GetConnectInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HConnect>(_connect);
}

/**
 * Get a handle to the Ecom Interface.
 * @return EOS_HEcom handle
 *
 * @see eos_ecom.h
 * @see eos_ecom_types.h
 */
EOS_HEcom              EOSSDK_Platform::GetEcomInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HEcom>(_ecom);
}

/**
 * Get a handle to the UI Interface.
 * @return EOS_HUI handle
 *
 * @see eos_ui.h
 * @see eos_ui_types.h
 */
EOS_HUI                EOSSDK_Platform::GetUIInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HUI>(_ui);
}

/**
 * Get a handle to the Friends Interface.
 * @return EOS_HFriends handle
 *
 * @see eos_friends.h
 * @see eos_friends_types.h
 */
EOS_HFriends           EOSSDK_Platform::GetFriendsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HFriends>(_friends);
}

/**
 * Get a handle to the Presence Interface.
 * @return EOS_HPresence handle
 *
 * @see eos_presence.h
 * @see eos_presence_types.h
 */
EOS_HPresence          EOSSDK_Platform::GetPresenceInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HPresence>(_presence);
}


/**
 * Get a handle to the Sessions Interface.
 * @return EOS_HSessions handle
 *
 * @see eos_sessions.h
 * @see eos_sessions_types.h
 */
EOS_HSessions          EOSSDK_Platform::GetSessionsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HSessions>(_sessions);
}

/**
 * Get a handle to the Lobby Interface.
 * @return EOS_HLobby handle
 *
 * @see eos_lobby.h
 * @see eos_lobby_types.h
 */
EOS_HLobby             EOSSDK_Platform::GetLobbyInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HLobby>(_lobby);
}

/**
 * Get a handle to the UserInfo Interface.
 * @return EOS_HUserInfo handle
 *
 * @see eos_userinfo.h
 * @see eos_userinfo_types.h
 */
EOS_HUserInfo          EOSSDK_Platform::GetUserInfoInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HUserInfo>(_userinfo);
}

/**
 * Get a handle to the Peer-to-Peer Networking Interface.
 * @return EOS_HP2P handle
 *
 * @see eos_p2p.h
 * @see eos_p2p_types.h
 */
EOS_HP2P               EOSSDK_Platform::GetP2PInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HP2P>(_p2p);
}

/**
 * Get a handle to the PlayerDataStorage Interface.
 * @return EOS_HPlayerDataStorage handle
 *
 * @see eos_playerdatastorage.h
 * @see eos_playerdatastorage_types.h
 */
EOS_HPlayerDataStorage EOSSDK_Platform::GetPlayerDataStorageInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HPlayerDataStorage>(_playerdatastorage);
}

/**
 * Get a handle to the Achievements Interface.
 * @return EOS_HAchievements handle
 *
 * @see eos_achievements.h
 * @see eos_achievements_types.h
 */
EOS_HAchievements      EOSSDK_Platform::GetAchievementsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HAchievements>(_achievements);
}

/**
 * Get a handle to the Stats Interface.
 * @return EOS_HStats handle
 *
 * @see eos_stats.h
 * @see eos_stats_types.h
 */
EOS_HStats             EOSSDK_Platform::GetStatsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HStats>(_stats);
}

/**
 * Get a handle to the Leaderboards Interface.
 * @return EOS_HLeaderboards handle
 *
 * @see eos_leaderboards.h
 * @see eos_leaderboards_types.h
 */
EOS_HLeaderboards      EOSSDK_Platform::GetLeaderboardsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HLeaderboards>(_leaderboards);
}

/**
 * Get the active country code that the SDK will send to services which require it.
 * This only will return the value set as the override otherwise EOS_NotFound is returned.
 * This is not currently used for anything internally.
 *
 * @param LocalUserId The account to use for lookup if no override exists.
 * @param OutBuffer The buffer into which the character data should be written.  The buffer must be long enough to hold a string of EOS_COUNTRYCODE_MAX_LENGTH.
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the active country code string was copied into the OutBuffer.
 *         EOS_Success if the information is available and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if there is neither an override nor an available country code for the user.
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the country code string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 *
 * @see eos_ecom.h
 * @see EOS_COUNTRYCODE_MAX_LENGTH
 */
EOS_EResult EOSSDK_Platform::GetActiveCountryCode(EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();

    if (OutBuffer == nullptr || InOutBufferLength == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (*InOutBufferLength < (static_strlen("") + 1))
        return EOS_EResult::EOS_LimitExceeded;

    strncpy(OutBuffer, "", static_strlen("") + 1);

    return EOS_EResult::EOS_Success;
}

/**
 * Get the active locale code that the SDK will send to services which require it.
 * This returns the override value otherwise it will use the locale code of the given user.
 * This is used for localization. This follows ISO 639.
 *
 * @param LocalUserId The account to use for lookup if no override exists.
 * @param OutBuffer The buffer into which the character data should be written.  The buffer must be long enough to hold a string of EOS_LOCALECODE_MAX_LEN.
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the active locale code string was copied into the OutBuffer.
 *         EOS_Success if the information is available and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if there is neither an override nor an available locale code for the user.
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the locale code string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 *
 * @see eos_ecom.h
 * @see EOS_LOCALECODE_MAX_LEN
 */
EOS_EResult EOSSDK_Platform::GetActiveLocaleCode(EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();

    if (OutBuffer == nullptr || InOutBufferLength == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (*InOutBufferLength < (static_strlen("en") + 1))
        return EOS_EResult::EOS_LimitExceeded;

    strncpy(OutBuffer, "en", static_strlen("en") + 1);

    return EOS_EResult::EOS_Success;
}

/**
 * Get the override country code that the SDK will send to services which require it.
 * This is currently used for determining pricing.
 *
 * @param OutBuffer The buffer into which the character data should be written.  The buffer must be long enough to hold a string of EOS_COUNTRYCODE_MAX_LEN.
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the override country code string was copied into the OutBuffer.
 *         EOS_Success if the information is available and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the country code string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 *
 * @see eos_ecom.h
 * @see EOS_COUNTRYCODE_MAX_LEN
 */
EOS_EResult EOSSDK_Platform::GetOverrideCountryCode(char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();

    if (OutBuffer == nullptr || InOutBufferLength == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (*InOutBufferLength < (_override_country_code.length() + 1))
        return EOS_EResult::EOS_LimitExceeded;

    strncpy(OutBuffer, _override_country_code.c_str(), _override_country_code.length() + 1);

    return EOS_EResult::EOS_Success;
}

/**
 * Get the override locale code that the SDK will send to services which require it.
 * This is used for localization. This follows ISO 639.
 *
 * @param OutBuffer The buffer into which the character data should be written.  The buffer must be long enough to hold a string of EOS_LOCALECODE_MAX_LEN.
 * @param InOutBufferLength The size of the OutBuffer in characters.
 *                          The input buffer should include enough space to be null-terminated.
 *                          When the function returns, this parameter will be filled with the length of the string copied into OutBuffer.
 *
 * @return An EOS_EResult that indicates whether the override locale code string was copied into the OutBuffer.
 *         EOS_Success if the information is available and passed out in OutBuffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_LimitExceeded - The OutBuffer is not large enough to receive the locale code string. InOutBufferLength contains the required minimum length to perform the operation successfully.
 *
 * @see eos_ecom.h
 * @see EOS_LOCALECODE_MAX_LEN
 */
EOS_EResult EOSSDK_Platform::GetOverrideLocaleCode(char* OutBuffer, int32_t* InOutBufferLength)
{
    TRACE_FUNC();

    if (OutBuffer == nullptr || InOutBufferLength == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    if (*InOutBufferLength < (_override_locale_code.length() + 1))
        return EOS_EResult::EOS_LimitExceeded;

    strncpy(OutBuffer, _override_locale_code.c_str(), _override_locale_code.length() + 1);

    return EOS_EResult::EOS_Success;
}

/**
 * Set the override country code that the SDK will send to services which require it.
 * This is currently used for determining accurate pricing.
 *
 * @return An EOS_EResult that indicates whether the override country code string was saved.
 *         EOS_Success if the country code was overridden
 *         EOS_InvalidParameters if you pass an invalid country code
 *
 * @see eos_ecom.h
 * @see EOS_COUNTRYCODE_MAX_LEN
 */
EOS_EResult EOSSDK_Platform::SetOverrideCountryCode(const char* NewCountryCode)
{
    TRACE_FUNC();

    if (NewCountryCode == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    LOG(Log::LogLevel::DEBUG, "%s", NewCountryCode);

    _override_country_code = NewCountryCode;

    return EOS_EResult::EOS_Success;
}

/**
 * Set the override locale code that the SDK will send to services which require it.
 * This is used for localization. This follows ISO 639.
 *
 * @return An EOS_EResult that indicates whether the override locale code string was saved.
 *         EOS_Success if the locale code was overridden
 *         EOS_InvalidParameters if you pass an invalid locale code
 *
 * @see eos_ecom.h
 * @see EOS_LOCALECODE_MAX_LEN
 */
EOS_EResult EOSSDK_Platform::SetOverrideLocaleCode(const char* NewLocaleCode)
{
    TRACE_FUNC();

    if (NewLocaleCode == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    LOG(Log::LogLevel::DEBUG, "%s", NewLocaleCode);

    _override_locale_code = NewLocaleCode;

    return EOS_EResult::EOS_Success;
}

/**
 * Checks if the app was launched through the Epic Launcher, and relaunches it through the Epic Launcher if it wasn't.
 *
 * @return An EOS_EResult is returned to indicate success or an error.
 *
 * EOS_Success is returned if the app is being restarted. You should quit your process as soon as possible.
 * EOS_NoChange is returned if the app was already launched through the Epic Launcher, and no action needs to be taken.
 * EOS_UnexpectedError is returned if the LauncherCheck module failed to initialize, or the module tried and failed to restart the app.
 */
EOS_EResult EOSSDK_Platform::CheckForLauncherAndRestart()
{
    TRACE_FUNC();

    return EOS_EResult::EOS_NoChange;
}

}