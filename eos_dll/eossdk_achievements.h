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

namespace sdk
{
    class EOSSDK_Achievements :
        public IRunFrame
    {
        static const std::string achievements_filename;
        static const std::string achievements_db_filename;

        fifo_json _achievements_db;
        fifo_json _achievements;
        std::unordered_map<std::string, fifo_json*> _unlocked_achievements;

    public:
        EOSSDK_Achievements();
        ~EOSSDK_Achievements();

        EOS_EResult copy_definition(typename decltype(_achievements_db)::iterator it, EOS_Achievements_Definition** OutDefinition);
        EOS_EResult copy_definition_v2(typename decltype(_achievements_db)::iterator it, EOS_Achievements_DefinitionV2** OutDefinition);
        EOS_EResult copy_unlocked_achievement(typename decltype(_unlocked_achievements)::iterator it, EOS_Achievements_UnlockedAchievement** OutAchievement);
        EOS_EResult copy_player_achievement(typename decltype(_achievements)::iterator it, EOS_Achievements_PlayerAchievement** OutAchievement);

        // RunFrame is always called when running callbacks
        virtual bool CBRunFrame();
        // RunNetwork is run if you register to a network message and we received that message
        virtual bool RunNetwork(Network_Message_pb const& msg);
        // RunCallbacks is run when you sent a callback
        // True  = FrameResult_t has been filled with a result
        // False = FrameResult_t is not changed
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void               QueryDefinitions(const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate);
        uint32_t           GetAchievementDefinitionCount(const EOS_Achievements_GetAchievementDefinitionCountOptions* Options);
        EOS_EResult        CopyAchievementDefinitionV2ByIndex(const EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition);
        EOS_EResult        CopyAchievementDefinitionV2ByAchievementId(const EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition);
        void               QueryPlayerAchievements(const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate);
        uint32_t           GetPlayerAchievementCount(const EOS_Achievements_GetPlayerAchievementCountOptions* Options);
        EOS_EResult        CopyPlayerAchievementByIndex(const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement);
        EOS_EResult        CopyPlayerAchievementByAchievementId(const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement);
        void               UnlockAchievements(const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate);
        uint32_t           GetUnlockedAchievementCount(const EOS_Achievements_GetUnlockedAchievementCountOptions* Options);
        EOS_EResult        CopyUnlockedAchievementByIndex(const EOS_Achievements_CopyUnlockedAchievementByIndexOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement);
        EOS_EResult        CopyUnlockedAchievementByAchievementId(const EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement);
        EOS_NotificationId AddNotifyAchievementsUnlocked(const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn);
        EOS_NotificationId AddNotifyAchievementsUnlockedV2(const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn);
        void               RemoveNotifyAchievementsUnlocked(EOS_NotificationId InId);
        EOS_EResult        CopyAchievementDefinitionByIndex(const EOS_Achievements_CopyAchievementDefinitionByIndexOptions* Options, EOS_Achievements_Definition** OutDefinition);
        EOS_EResult        CopyAchievementDefinitionByAchievementId(const EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions* Options, EOS_Achievements_Definition** OutDefinition);
    };
}