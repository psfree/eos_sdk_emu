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

#include "eos_platform.h"

namespace sdk
{

EOS_Platform::EOS_Platform():
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

EOS_Platform::~EOS_Platform()
{
    Release();
    delete _network;
    delete _cb_manager;
}

EOS_Platform& EOS_Platform::Inst()
{
    static EOS_Platform instance;
    return instance;
}

void EOS_Platform::Init(const EOS_Platform_Options* options)
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
                }
            }
        }

        _metrics           = new EOS_Metrics;
        _auth              = new EOS_Auth;
        _connect           = new EOS_Connect;
        _ecom              = new EOS_Ecom;
        _ui                = new EOS_UI;
        _friends           = new EOS_Friends;
        _presence          = new EOS_Presence;
        _sessions          = new EOS_Sessions;
        _lobby             = new EOS_Lobby;
        _userinfo          = new EOS_UserInfo;
        _p2p               = new EOS_P2P;
        _playerdatastorage = new EOS_PlayerDataStorage;
        _achievements      = new EOS_Achievements;
        _stats             = new EOS_Stats;
        _leaderboards      = new EOS_Leaderboards;

        _platform_init = true;
    }
}

void EOS_Platform::Release()
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

void EOS_Platform::Tick()
{
    GLOBAL_LOCK();
    GetCB_Manager().run_frames();
    GetCB_Manager().run_callbacks();
}

EOS_HMetrics           EOS_Platform::GetMetricsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HMetrics>(_metrics);
}

EOS_HAuth              EOS_Platform::GetAuthInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HAuth>(_auth);
}

EOS_HConnect           EOS_Platform::GetConnectInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HConnect>(_connect);
}

EOS_HEcom              EOS_Platform::GetEcomInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HEcom>(_ecom);
}

EOS_HUI                EOS_Platform::GetUIInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HUI>(_ui);
}

EOS_HFriends           EOS_Platform::GetFriendsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HFriends>(_friends);
}

EOS_HPresence          EOS_Platform::GetPresenceInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HPresence>(_presence);
}

EOS_HSessions          EOS_Platform::GetSessionsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HSessions>(_sessions);
}

EOS_HLobby             EOS_Platform::GetLobbyInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HLobby>(_lobby);
}

EOS_HUserInfo          EOS_Platform::GetUserInfoInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HUserInfo>(_userinfo);
}

EOS_HP2P               EOS_Platform::GetP2PInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HP2P>(_p2p);
}

EOS_HPlayerDataStorage EOS_Platform::GetPlayerDataStorageInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HPlayerDataStorage>(_playerdatastorage);
}

EOS_HAchievements      EOS_Platform::GetAchievementsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HAchievements>(_achievements);
}

EOS_HStats             EOS_Platform::GetStatsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HStats>(_stats);
}

EOS_HLeaderboards      EOS_Platform::GetLeaderboardsInterface()
{
    GLOBAL_LOCK();
    return reinterpret_cast<EOS_HLeaderboards>(_leaderboards);
}

}