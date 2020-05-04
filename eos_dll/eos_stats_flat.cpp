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

#include "eos_stats.h"

using namespace sdk;

/**
 * Ingest a stat by the amount specified in Options.
 * When the operation is complete and the delegate is triggered the stat will be uploaded to the backend to be processed.
 * The stat may not be updated immediately and an achievement using the stat may take a while to be unlocked once the stat has been uploaded.
 *
 * @param Options Structure containing information about the stat we're ingesting.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the ingest stat operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Stats_IngestStat(EOS_HStats Handle, const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Stats*>(Handle);
}

/**
 * Query for a list of stats for a specific player.
 *
 * @param Options Structure containing information about the player whose stats we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate This function is called when the query player stats operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Stats_QueryStats(EOS_HStats Handle, const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Stats*>(Handle);
}

/**
 * Fetch the number of stats that are cached locally.
 *
 * @param Options The Options associated with retrieving the stat count
 *
 * @see EOS_Stats_CopyStatByIndex
 *
 * @return Number of stats or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Stats_GetStatsCount(EOS_HStats Handle, const EOS_Stats_GetStatCountOptions* Options)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Stats*>(Handle);
    return 0;
}

/**
 * Fetches a stat from a given index.
 *
 * @param Options Structure containing the account id and index being accessed
 * @param OutAchievement The stat data for the given index, if it exists and is valid, use EOS_Stats_Stat_Release when finished
 *
 * @see EOS_Stats_Stat_Release
 *
 * @return EOS_Success if the information is available and passed out in OutStat
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the stat is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByIndex(EOS_HStats Handle, const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat** OutStat)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Stats*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Fetches a stat from cached stats by name.
 *
 * @param Options Structure containing the account id and name being accessed
 * @param OutAchievement The stat data for the given name, if it exists and is valid, use EOS_Stats_Stat_Release when finished
 *
 * @see EOS_Stats_Stat_Release
 *
 * @return EOS_Success if the information is available and passed out in OutStat
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the stat is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByName(EOS_HStats Handle, const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat** OutStat)
{
    LOG(Log::LogLevel::TRACE, "");

    auto pInst = reinterpret_cast<EOS_Stats*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Release the memory associated with a stat. This must be called on data retrieved from EOS_Stats_CopyStatByIndex or EOS_Stats_CopyStatByName.
 *
 * @param Stat - The stat data to release.
 *
 * @see EOS_Stats_Stat
 * @see EOS_Stats_CopyStatByIndex
 * @see EOS_Stats_CopyStatByName
 */
EOS_DECLARE_FUNC(void) EOS_Stats_Stat_Release(EOS_Stats_Stat* Stat)
{

}