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

#include "eossdk_leaderboards.h"
#include "eossdk_platform.h"

namespace sdk
{

EOSSDK_Leaderboards::EOSSDK_Leaderboards()
{}

EOSSDK_Leaderboards::~EOSSDK_Leaderboards()
{}

/**
 * The following EOS_Leaderboards_* functions allow you to query existing leaderboard definitions that have been defined for your application.
 * You can retrieve a list of scores for the top users for each Leaderboard.
 * You can also query scores for one or more users.
 */

 /**
  * Query for a list of existing leaderboards definitions including their attributes.
  *
  * @param Options Structure containing information about the application whose leaderboard definitions we're retrieving.
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
  * @param CompletionDelegate This function is called when the query operation completes.
  *
  * @return EOS_Success if the operation completes successfully
  *         EOS_InvalidParameters if any of the options are incorrect
  */
void EOSSDK_Leaderboards::QueryLeaderboardDefinitions(const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo& oqldcci = res->CreateCallback<EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);
    oqldcci.ClientData = ClientData;

    if (Options == nullptr)
    {
        oqldcci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        oqldcci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of leaderboards definitions that are cached locally.
 *
 * @param Options The Options associated with retrieving the leaderboard count.
 *
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByIndex
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId
 *
 * @return Number of leaderboards or 0 if there is an error
 */
uint32_t EOSSDK_Leaderboards::GetLeaderboardDefinitionCount(const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return 0;

    return 0;
}

/**
 * Fetches a leaderboard definition from the cache using an index.
 *
 * @param Options Structure containing the index being accessed.
 * @param OutLeaderboardDefinition The leaderboard data for the given index, if it exists and is valid, use EOS_Leaderboards_LeaderboardDefinition_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardDefinition_Release
 *
 * @return EOS_Success if the information is available and passed out in OutLeaderboardDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard is not found
 */
EOS_EResult EOSSDK_Leaderboards::CopyLeaderboardDefinitionByIndex(const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    TRACE_FUNC();

    if (Options == nullptr || OutLeaderboardDefinition == nullptr)
    {
        set_nullptr(OutLeaderboardDefinition);
        return EOS_EResult::EOS_InvalidParameters;
    }

    OutLeaderboardDefinition = nullptr;
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetches a leaderboard definition from the cache using a leaderboard ID.
 *
 * @param Options Structure containing the leaderboard ID being accessed.
 * @param OutLeaderboardDefinition The leaderboard definition for the given leaderboard ID, if it exists and is valid, use EOS_Leaderboards_LeaderboardDefinition_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardDefinition_Release
 *
 * @return EOS_Success if the information is available and passed out in OutLeaderboardDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard data is not found
 */
EOS_EResult EOSSDK_Leaderboards::CopyLeaderboardDefinitionByLeaderboardId(const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->LeaderboardId == nullptr || OutLeaderboardDefinition == nullptr)
    {
        set_nullptr(OutLeaderboardDefinition);
        return EOS_EResult::EOS_InvalidParameters;
    }

    OutLeaderboardDefinition = nullptr;
    return EOS_EResult::EOS_NotFound;
}

/**
 * Retrieves top leaderboard records by rank in the leaderboard matching the given leaderboard ID.
 *
 * @param Options Structure containing information about the leaderboard records we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the query operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_Leaderboards::QueryLeaderboardRanks(const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo& oqlrcci = res->CreateCallback<EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);
    oqlrcci.ClientData = ClientData;

    if (Options == nullptr)
    {
        oqlrcci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        oqlrcci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of leaderboard records that are cached locally.
 *
 * @param Options The Options associated with retrieving the leaderboard record count.
 *
 * @see EOS_Leaderboards_CopyLeaderboardRecordByIndex
 * @see EOS_Leaderboards_CopyLeaderboardRecordByUserId
 *
 * @return Number of leaderboard records or 0 if there is an error
 */
uint32_t EOSSDK_Leaderboards::GetLeaderboardRecordCount(const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
    {
        return 0;
    }

    return 0;
}

/**
 * Fetches a leaderboard record from a given index.
 *
 * @param Options Structure containing the index being accessed.
 * @param OutLeaderboardRecord The leaderboard record for the given index, if it exists and is valid, use EOS_Leaderboards_LeaderboardRecord_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardRecord_Release
 *
 * @return EOS_Success if the leaderboard record is available and passed out in OutLeaderboardRecord
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard is not found
 */
EOS_EResult EOSSDK_Leaderboards::CopyLeaderboardRecordByIndex(const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    TRACE_FUNC();

    if (Options == nullptr || OutLeaderboardRecord == nullptr)
    {
        set_nullptr(OutLeaderboardRecord);
        return EOS_EResult::EOS_InvalidParameters;
    }

    OutLeaderboardRecord = nullptr;
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetches a leaderboard record from a given user ID.
 *
 * @param Options Structure containing the user ID being accessed.
 * @param OutLeaderboardRecord The leaderboard record for the given user ID, if it exists and is valid, use EOS_Leaderboards_LeaderboardRecord_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardRecord_Release
 *
 * @return EOS_Success if the leaderboard record is available and passed out in OutLeaderboardRecord
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard data is not found
 */
EOS_EResult EOSSDK_Leaderboards::CopyLeaderboardRecordByUserId(const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->UserId == nullptr || OutLeaderboardRecord == nullptr)
    {
        set_nullptr(OutLeaderboardRecord);
        return EOS_EResult::EOS_InvalidParameters;
    }

    OutLeaderboardRecord = nullptr;
    return EOS_EResult::EOS_NotFound;
}

/**
 * Query for a list of scores for a given list of users.
 *
 * @param Options Structure containing information about the users whose scores we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the query operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_Leaderboards::QueryLeaderboardUserScores(const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo& oqluscci = res->CreateCallback<EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);
    oqluscci.ClientData = ClientData;

    if (Options == nullptr)
    {
        oqluscci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        oqluscci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of leaderboard user scores that are cached locally.
 *
 * @param Options The Options associated with retrieving the leaderboard user scores count.
 *
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByIndex
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByUserId
 *
 * @return Number of leaderboard records or 0 if there is an error
 */
uint32_t EOSSDK_Leaderboards::GetLeaderboardUserScoreCount(const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options)
{
    TRACE_FUNC();

    if (Options == nullptr)
        return 0;

    return 0;
}

/**
 * Fetches leaderboard user score from a given index.
 *
 * @param Options Structure containing the index being accessed.
 * @param OutLeaderboardUserScore The leaderboard user score for the given index, if it exists and is valid, use EOS_Leaderboards_LeaderboardUserScore_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardUserScore_Release
 *
 * @return EOS_Success if the leaderboard scores are available and passed out in OutLeaderboardUserScore
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard user scores are not found
 */
EOS_EResult EOSSDK_Leaderboards::CopyLeaderboardUserScoreByIndex(const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    TRACE_FUNC();

    if (Options == nullptr || OutLeaderboardUserScore == nullptr)
    {
        set_nullptr(OutLeaderboardUserScore);
        return EOS_EResult::EOS_InvalidParameters;
    }

    OutLeaderboardUserScore = nullptr;
    return EOS_EResult::EOS_NotFound;
}

/**
 * Fetches leaderboard user score from a given user ID.
 *
 * @param Options Structure containing the user ID being accessed.
 * @param OutLeaderboardUserScore The leaderboard user score for the given user ID, if it exists and is valid, use EOS_Leaderboards_LeaderboardUserScore_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardUserScore_Release
 *
 * @return EOS_Success if the leaderboard scores are available and passed out in OutLeaderboardUserScore
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard user scores are not found
 */
EOS_EResult EOSSDK_Leaderboards::CopyLeaderboardUserScoreByUserId(const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->StatName == nullptr || OutLeaderboardUserScore == nullptr)
    {
        set_nullptr(OutLeaderboardUserScore);
        return EOS_EResult::EOS_InvalidParameters;
    }

    OutLeaderboardUserScore = nullptr;
    return EOS_EResult::EOS_NotFound;
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Leaderboards::CBRunFrame()
{
    return false;
}

bool EOSSDK_Leaderboards::RunCallbacks(pFrameResult_t res)
{
    GLOBAL_LOCK();

    return res->done;
}

void EOSSDK_Leaderboards::FreeCallback(pFrameResult_t res)
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