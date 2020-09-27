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

namespace sdk
{
    class EOSSDK_Stats :
        public IRunCallback
    {
        static const std::string stats_filename;

        nlohmann::json _stats;

    public:
        EOSSDK_Stats();
        ~EOSSDK_Stats();

        void save_stats();

        virtual bool CBRunFrame();
        virtual bool RunCallbacks(pFrameResult_t res);
        virtual void FreeCallback(pFrameResult_t res);

        void        IngestStat(const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate);
        void        QueryStats(const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate);
        uint32_t    GetStatsCount(const EOS_Stats_GetStatCountOptions* Options);
        EOS_EResult CopyStatByIndex(const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat** OutStat);
        EOS_EResult CopyStatByName(const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat** OutStat);
    };
}