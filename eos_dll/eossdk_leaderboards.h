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
#include "callback_manager.h"
#include "network.h"

namespace sdk
{
    class EOSSDK_Leaderboards :
        public IRunCallback
    {
    public:
        EOSSDK_Leaderboards();
        ~EOSSDK_Leaderboards();

        virtual bool CBRunFrame();
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void        QueryLeaderboardDefinitions(const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate);
        uint32_t    GetLeaderboardDefinitionCount(const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options);
        EOS_EResult CopyLeaderboardDefinitionByIndex(const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition);
        EOS_EResult CopyLeaderboardDefinitionByLeaderboardId(const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition);
        void        QueryLeaderboardRanks(const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate);
        uint32_t    GetLeaderboardRecordCount(const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options);
        EOS_EResult CopyLeaderboardRecordByIndex(const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord);
        EOS_EResult CopyLeaderboardRecordByUserId(const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord);
        void        QueryLeaderboardUserScores(const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate);
        uint32_t    GetLeaderboardUserScoreCount(const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options);
        EOS_EResult CopyLeaderboardUserScoreByIndex(const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore);
        EOS_EResult CopyLeaderboardUserScoreByUserId(const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore);
    };
}