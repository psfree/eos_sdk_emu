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

#pragma once

#include "common_includes.h"

#include "network.h"

#include "eossdk_metrics.h"
#include "eossdk_auth.h"
#include "eossdk_connect.h"
#include "eossdk_ecom.h"
#include "eossdk_ui.h"
#include "eossdk_friends.h"
#include "eossdk_presence.h"
#include "eossdk_sessions.h"
#include "eossdk_lobby.h"
#include "eossdk_userinfo.h"
#include "eossdk_p2p.h"
#include "eossdk_playerdatastorage.h"
#include "eossdk_achievements.h"
#include "eossdk_stats.h"
#include "eossdk_leaderboards.h"

namespace sdk
{
    class EOS_Platform
    {
        EOS_Platform();

        bool _platform_init;

    public:
        int32_t     _api_version;
        void*       _reserved;
        std::string _product_id;
        std::string _sandbox_id;
        std::string _client_id;
        std::string _client_secret;
        EOS_Bool    _is_server;
        std::string _encryption_key;
        std::string _override_country_code;
        std::string _override_locale_code;
        std::string _deployment_id;
        uint64_t    _flags;
        std::string _cache_directory;

        Callback_Manager      *_cb_manager;
        Network               *_network;

        EOSSDK_Metrics           *_metrics;
        EOSSDK_Auth              *_auth;
        EOSSDK_Connect           *_connect;
        EOSSDK_Ecom              *_ecom;
        EOSSDK_UI                *_ui;
        EOSSDK_Friends           *_friends;
        EOSSDK_Presence          *_presence;
        EOSSDK_Sessions          *_sessions;
        EOSSDK_Lobby             *_lobby;
        EOSSDK_UserInfo          *_userinfo;
        EOSSDK_P2P               *_p2p;
        EOSSDK_PlayerDataStorage *_playerdatastorage;
        EOSSDK_Achievements      *_achievements;
        EOSSDK_Stats             *_stats;
        EOSSDK_Leaderboards      *_leaderboards;

        ~EOS_Platform();

        static EOS_Platform& Inst();
        void Init(const EOS_Platform_Options* options);

        void Release();
        void Tick();
        EOS_HMetrics           GetMetricsInterface();
        EOS_HAuth              GetAuthInterface();
        EOS_HConnect           GetConnectInterface();
        EOS_HEcom              GetEcomInterface();
        EOS_HUI                GetUIInterface();
        EOS_HFriends           GetFriendsInterface();
        EOS_HPresence          GetPresenceInterface();
        EOS_HSessions          GetSessionsInterface();
        EOS_HLobby             GetLobbyInterface();
        EOS_HUserInfo          GetUserInfoInterface();
        EOS_HP2P               GetP2PInterface();
        EOS_HPlayerDataStorage GetPlayerDataStorageInterface();
        EOS_HAchievements      GetAchievementsInterface();
        EOS_HStats             GetStatsInterface();
        EOS_HLeaderboards      GetLeaderboardsInterface();
    };
}

inline sdk::EOS_Platform&          GetEOS_Platform         () { return  sdk::EOS_Platform::Inst();            }

inline Callback_Manager&           GetCB_Manager           () { return *GetEOS_Platform()._cb_manager;        }
inline Network&                    GetNetwork              () { return *GetEOS_Platform()._network;           }

inline sdk::EOSSDK_Metrics&           GetEOS_Metrics          () { return *GetEOS_Platform()._metrics;           }
inline sdk::EOSSDK_Auth&              GetEOS_Auth             () { return *GetEOS_Platform()._auth;              }
inline sdk::EOSSDK_Connect&           GetEOS_Connect          () { return *GetEOS_Platform()._connect;           }
inline sdk::EOSSDK_Ecom&              GetEOS_Ecom             () { return *GetEOS_Platform()._ecom;              }
inline sdk::EOSSDK_UI&                GetEOS_UI               () { return *GetEOS_Platform()._ui;                }
inline sdk::EOSSDK_Friends&           GetEOS_Friends          () { return *GetEOS_Platform()._friends;           }
inline sdk::EOSSDK_Presence&          GetEOS_Presence         () { return *GetEOS_Platform()._presence;          }
inline sdk::EOSSDK_Sessions&          GetEOS_Sessions         () { return *GetEOS_Platform()._sessions;          }
inline sdk::EOSSDK_Lobby&             GetEOS_Lobby            () { return *GetEOS_Platform()._lobby;             }
inline sdk::EOSSDK_UserInfo&          GetEOS_UserInfo         () { return *GetEOS_Platform()._userinfo;          }
inline sdk::EOSSDK_P2P&               GetEOS_P2P              () { return *GetEOS_Platform()._p2p;               }
inline sdk::EOSSDK_PlayerDataStorage& GetEOS_PlayerDataStorage() { return *GetEOS_Platform()._playerdatastorage; }
inline sdk::EOSSDK_Achievements&      GetEOS_Achievements     () { return *GetEOS_Platform()._achievements;      }
inline sdk::EOSSDK_Stats&             GetEOS_Stats            () { return *GetEOS_Platform()._stats;             }
inline sdk::EOSSDK_Leaderboards&      GetEOS_Leaderboards     () { return *GetEOS_Platform()._leaderboards;      }