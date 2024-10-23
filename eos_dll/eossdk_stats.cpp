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

#include "eossdk_stats.h"
#include "eossdk_platform.h"

namespace sdk
{

decltype(EOSSDK_Stats::stats_filename) EOSSDK_Stats::stats_filename("stats.json");

EOSSDK_Stats::EOSSDK_Stats()
{
    FileManager::load_json(stats_filename, _stats);
}

EOSSDK_Stats::~EOSSDK_Stats()
{}

void EOSSDK_Stats::save_stats()
{
    FileManager::save_json(stats_filename, _stats);
}

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
void EOSSDK_Stats::IngestStat(const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Stats_IngestStatCompleteCallbackInfo& iscci = res->CreateCallback<EOS_Stats_IngestStatCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);
    iscci.ClientData = ClientData;
    if (Options == nullptr)
    {
        iscci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {

        switch (Options->ApiVersion) {
            case EOS_STATS_INGESTSTAT_API_003:
            case EOS_STATS_INGESTSTAT_API_002:
            {
                iscci.TargetUserId = Options->TargetUserId;
                iscci.UserId = Options->LocalUserId;
            }
            case EOS_STATS_INGESTSTAT_API_001:
            {
                const EOS_Stats_IngestStatOptions001* opts = reinterpret_cast<const EOS_Stats_IngestStatOptions001*>(Options);
                iscci.TargetUserId = opts->UserId;
                iscci.UserId = opts->UserId;
            }
        }
        

        if (Options->StatsCount > EOS_STATS_MAX_INGEST_STATS || Options->Stats == nullptr)
        {
            iscci.ResultCode = EOS_EResult::EOS_InvalidParameters;
        }
        else
        {
            for (int i = 0; i < Options->StatsCount; ++i)
            {
                auto& stat = Options->Stats[i];
                _stats[stat.StatName] += stat.IngestAmount;
            }

            save_stats();
            iscci.ResultCode = EOS_EResult::EOS_Success;
        }
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Query for a list of stat for a specific player.
 *
 * @param Options Structure containing information about the player whose stat we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate This function is called when the query player stat operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_Stats::QueryStats(const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);

    EOS_Stats_OnQueryStatsCompleteCallbackInfo& oqscci = res->CreateCallback<EOS_Stats_OnQueryStatsCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);
    oqscci.ClientData = ClientData;
    if (Options == nullptr)
    {
        oqscci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        oqscci.ResultCode = EOS_EResult::EOS_Success;
    }
    oqscci.UserId = Options->UserId;

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of stat that are cached locally.
 *
 * @param Options The Options associated with retrieving the stat count
 *
 * @see EOS_Stats_CopyStatByIndex
 *
 * @return Number of stat or 0 if there is an error
 */
uint32_t EOSSDK_Stats::GetStatsCount(const EOS_Stats_GetStatCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return 0;

    return _stats.size();
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
EOS_EResult EOSSDK_Stats::CopyStatByIndex(const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat** OutStat)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->StatIndex > _stats.size() || OutStat == nullptr)
    {
        set_nullptr(OutStat);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _stats.begin();
    std::advance(it, Options->StatIndex);

    EOS_Stats_Stat* stat = new EOS_Stats_Stat();
    stat->ApiVersion = EOS_STATS_STAT_API_LATEST;
    stat->StartTime = EOS_STATS_TIME_UNDEFINED;
    stat->EndTime = EOS_STATS_TIME_UNDEFINED;
    {
        size_t len = it.key().length() + 1;
        char* str = new char[len];
        strncpy(str, it.key().c_str(), len);
        stat->Name = str;
    }
    try
    {
        stat->Value = it.value();
    }
    catch(...)
    {
        stat->Value = 0;
        it.value() = 0;
    }

    *OutStat = stat;
    return EOS_EResult::EOS_Success;
}

/**
 * Fetches a stat from cached stat by name.
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
EOS_EResult EOSSDK_Stats::CopyStatByName(const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat** OutStat)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->Name == nullptr || OutStat == nullptr)
    {
        set_nullptr(OutStat);
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _stats.find(Options->Name);
    if (it == _stats.end())
    {
        set_nullptr(OutStat);
        return EOS_EResult::EOS_InvalidParameters;
    }

    EOS_Stats_Stat* stat = new EOS_Stats_Stat();
    stat->ApiVersion = EOS_STATS_STAT_API_LATEST;
    stat->StartTime = EOS_STATS_TIME_UNDEFINED;
    stat->EndTime = EOS_STATS_TIME_UNDEFINED;
    {
        size_t len = it.key().length() + 1;
        char* str = new char[len];
        strncpy(str, it.key().c_str(), len);
        stat->Name = str;
    }
    try
    {
        stat->Value = it.value();
    }
    catch (...)
    {
        stat->Value = 0;
        it.value() = 0;
    }

    *OutStat = stat;
    return EOS_EResult::EOS_Success;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Stats::CBRunFrame()
{
    return false;
}

bool EOSSDK_Stats::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Stats::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    //switch (res->res.m_iCallback)
    {
        /////////////////////////////
        //        Callbacks        //
        /////////////////////////////
        //case callback_type::k_iCallback:
        //{
        //    callback_type& callback = res->GetCallback<callback_type>();
        //    // Free resources
        //}
        //break;
        /////////////////////////////
        //      Notifications      //
        /////////////////////////////
        //case notification_type::k_iCallback:
        //{
        //    notification_type& callback = res->GetCallback<notification_type>();
        //    // Free resources
        //}
        //break;
    }
}

}