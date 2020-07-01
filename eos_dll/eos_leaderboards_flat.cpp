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

using namespace sdk;

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
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardDefinitions(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
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
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardDefinitionCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition** OutLeaderboardDefinition)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
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
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord** OutLeaderboardRecord)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
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
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardUserScoreCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore** OutLeaderboardUserScore)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Leaderboards*>(Handle);
    return EOS_EResult::EOS_Success;
}

/**
 * Release the memory associated with a leaderboard definition. This must be called on data retrieved from
 * EOS_Leaderboards_CopyLeaderboardDefinitionByIndex or EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId.
 *
 * @param LeaderboardDefinition - The Leaderboard definition to release.
 *
 * @see EOS_Leaderboards_LeaderboardDefinition
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByIndex
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardDefinition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition)
{
    EOS_Leaderboards_Definition_Release(LeaderboardDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_Definition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition)
{
    TRACE_FUNC();
}

/**
 * Release the memory associated with leaderboard user score. This must be called on data retrieved from
 * EOS_Leaderboards_CopyLeaderboardUserScoreByIndex or EOS_Leaderboards_CopyLeaderboardUserScoreByUserId.
 *
 * @param LeaderboardUserScore - The Leaderboard user score to release.
 *
 * @see EOS_Leaderboards_LeaderboardUserScore
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByIndex
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByUserId
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardUserScore_Release(EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore)
{
    TRACE_FUNC();

}

/**
 * Release the memory associated with leaderboard record. This must be called on data retrieved from
 * EOS_Leaderboards_CopyLeaderboardRecordByIndex or EOS_Leaderboards_CopyLeaderboardRecordByUserId.
 *
 * @param LeaderboardRecord - The Leaderboard record to release.
 *
 * @see EOS_Leaderboards_LeaderboardRecord
 * @see EOS_Leaderboards_CopyLeaderboardRecordByIndex
 * @see EOS_Leaderboards_CopyLeaderboardRecordByUserId
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardRecord_Release(EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord)
{
    TRACE_FUNC();

}