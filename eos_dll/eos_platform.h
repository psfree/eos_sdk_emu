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

#include "eos_metrics.h"
#include "eos_auth.h"
#include "eos_connect.h"
#include "eos_ecom.h"
#include "eos_ui.h"
#include "eos_friends.h"
#include "eos_presence.h"
#include "eos_sessions.h"
#include "eos_lobby.h"
#include "eos_userinfo.h"
#include "eos_p2p.h"
#include "eos_playerdatastorage.h"
#include "eos_achievements.h"
#include "eos_stats.h"
#include "eos_leaderboards.h"

namespace sdk
{
    class EOS_Platform
    {
        EOS_Platform();

        bool _platform_init;

    public:
        int32_t     _api_version;
        void* _reserved;
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

        EOS_Metrics           *_metrics;
        EOS_Auth              *_auth;
        EOS_Connect           *_connect;
        EOS_Ecom              *_ecom;
        EOS_UI                *_ui;
        EOS_Friends           *_friends;
        EOS_Presence          *_presence;
        EOS_Sessions          *_sessions;
        EOS_Lobby             *_lobby;
        EOS_UserInfo          *_userinfo;
        EOS_P2P               *_p2p;
        EOS_PlayerDataStorage *_playerdatastorage;
        EOS_Achievements      *_achievements;
        EOS_Stats             *_stats;
        EOS_Leaderboards      *_leaderboards;

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

inline sdk::EOS_Metrics&           GetEOS_Metrics          () { return *GetEOS_Platform()._metrics;           }
inline sdk::EOS_Auth&              GetEOS_Auth             () { return *GetEOS_Platform()._auth;              }
inline sdk::EOS_Connect&           GetEOS_Connect          () { return *GetEOS_Platform()._connect;           }
inline sdk::EOS_Ecom&              GetEOS_Ecom             () { return *GetEOS_Platform()._ecom;              }
inline sdk::EOS_UI&                GetEOS_UI               () { return *GetEOS_Platform()._ui;                }
inline sdk::EOS_Friends&           GetEOS_Friends          () { return *GetEOS_Platform()._friends;           }
inline sdk::EOS_Presence&          GetEOS_Presence         () { return *GetEOS_Platform()._presence;          }
inline sdk::EOS_Sessions&          GetEOS_Sessions         () { return *GetEOS_Platform()._sessions;          }
inline sdk::EOS_Lobby&             GetEOS_Lobby            () { return *GetEOS_Platform()._lobby;             }
inline sdk::EOS_UserInfo&          GetEOS_UserInfo         () { return *GetEOS_Platform()._userinfo;          }
inline sdk::EOS_P2P&               GetEOS_P2P              () { return *GetEOS_Platform()._p2p;               }
inline sdk::EOS_PlayerDataStorage& GetEOS_PlayerDataStorage() { return *GetEOS_Platform()._playerdatastorage; }
inline sdk::EOS_Achievements&      GetEOS_Achievements     () { return *GetEOS_Platform()._achievements;      }
inline sdk::EOS_Stats&             GetEOS_Stats            () { return *GetEOS_Platform()._stats;             }
inline sdk::EOS_Leaderboards&      GetEOS_Leaderboards     () { return *GetEOS_Platform()._leaderboards;      }