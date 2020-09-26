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

#include "eossdk_achievements.h"
#include "eossdk_platform.h"
#include "settings.h"

namespace sdk
{

decltype(EOSSDK_Achievements::achievements_filename)    EOSSDK_Achievements::achievements_filename("achievements.json");
decltype(EOSSDK_Achievements::achievements_db_filename) EOSSDK_Achievements::achievements_db_filename("achievements_db.json");

EOSSDK_Achievements::EOSSDK_Achievements()
{
    FileManager::load_json(achievements_filename, _achievements);
    FileManager::load_json(achievements_db_filename, _achievements_db);
}

EOSSDK_Achievements::~EOSSDK_Achievements()
{

    GetCB_Manager().remove_all_notifications(this);
}

EOS_EResult EOSSDK_Achievements::copy_definition(typename decltype(_achievements_db)::iterator it, EOS_Achievements_Definition** OutDefinition)
{
    EOS_Achievements_Definition* ach = new EOS_Achievements_Definition;
    memset(ach, 0, sizeof(*ach));

    ach->ApiVersion = EOS_ACHIEVEMENTS_DEFINITION_API_LATEST;

    try
    {
        ach->AchievementId = it.value()["achievement_id"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid achievement_id");
        goto achievement_error;
    }
    try
    {
        ach->DisplayName = it.value()["unlocked_display_name"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid unlocked_display_name for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        ach->Description = it.value()["unlocked_description"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid unlocked_description for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        ach->LockedDisplayName = it.value()["locked_display_name"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid locked_display_name for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        ach->LockedDescription = it.value()["locked_description"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid locked_description for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        try
        {
            ach->HiddenDescription = it.value()["hidden_description"].get_ref<std::string&>().c_str();
        }
        catch (...)
        {// If the user did not provide a hidden description (old field), use the locked description
            APP_LOG(Log::LogLevel::INFO, "No \"hidden_description\" in achievements_db, falling back to \"locked_description\"");
            ach->HiddenDescription = it.value()["locked_description"].get_ref<std::string&>().c_str();
        }
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid hidden_description or locked_description for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        try
        {
            ach->CompletionDescription = it.value()["completion_description"].get_ref<std::string&>().c_str();
        }
        catch (...)
        {// If the user did not provide a completion description (old field), use the flavor text
            APP_LOG(Log::LogLevel::INFO, "No \"completion_description\" in achievements_db, falling back to \"flavor_text\"");
            ach->CompletionDescription = it.value()["flavor_text"].get_ref<std::string&>().c_str();
        }
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid completion_description for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        ach->UnlockedIconId = it.value()["unlocked_icon_url"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid unlocked_icon_url for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        ach->LockedIconId = it.value()["locked_icon_url"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid locked_icon_url for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        ach->bIsHidden = (bool)it.value()["is_hidden"];
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid is_hidden for %s", ach->AchievementId);
        goto achievement_error;
    }
    try
    {
        auto& stats_thresholds = it.value()["stats_thresholds"];
        ach->StatThresholdsCount = stats_thresholds.size();
        EOS_Achievements_StatThresholds* stats = new EOS_Achievements_StatThresholds[ach->StatThresholdsCount];
        memset(stats, 0, sizeof(*stats) * ach->StatThresholdsCount);
        int i = 0;
        for (auto stat_it = stats_thresholds.begin(); stat_it != stats_thresholds.end(); ++stat_it, ++i)
        {
            stats[i].ApiVersion = EOS_ACHIEVEMENTS_STATTHRESHOLD_API_LATEST;
            try
            {
                stats[i].Name = stat_it.value()["name"].get_ref<std::string&>().c_str();
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds name for %s", ach->AchievementId);
                goto achievement_error;
            }
            try
            {
                stats[i].Threshold = stat_it.value()["threshold"];
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds[\"%s\"][\"threshold\"] for %s", stat_it.key().c_str(), ach->AchievementId);
                goto achievement_error;
            }
        }
        ach->StatThresholds = stats;
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds for %s", ach->AchievementId);
        goto achievement_error;
    }

    *OutDefinition = ach;
    return EOS_EResult::EOS_Success;

achievement_error:
    EOS_Achievements_Definition_Release(ach);

    *OutDefinition = nullptr;
    return EOS_EResult::EOS_UnexpectedError;
}

EOS_EResult EOSSDK_Achievements::copy_definition_v2(typename decltype(_achievements_db)::iterator it, EOS_Achievements_DefinitionV2** OutDefinition)
{
    EOS_Achievements_DefinitionV2* ach = new EOS_Achievements_DefinitionV2;
    memset(ach, 0, sizeof(*ach));
    ach->ApiVersion = EOS_ACHIEVEMENTS_DEFINITION_API_LATEST;

    try
    {
        ach->AchievementId = it.value()["achievement_id"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid Key");
        goto achievement_error;
    }
    try
    {
        ach->UnlockedDisplayName = it.value()["unlocked_display_name"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid unlocked_display_name for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->UnlockedDescription = it.value()["unlocked_description"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid unlocked_description for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->LockedDisplayName = it.value()["locked_display_name"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid locked_display_name for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->LockedDescription = it.value()["locked_description"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid locked_description for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->FlavorText = it.value()["flavor_text"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid flavor_text for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->UnlockedIconURL = it.value()["unlocked_icon_url"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid unlocked_icon_url for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->LockedIconURL = it.value()["locked_icon_url"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid locked_icon_url for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->bIsHidden = (bool)it.value()["is_hidden"];
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid is_hidden for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        auto& stats_thresholds = it.value()["stats_thresholds"];
        ach->StatThresholdsCount = stats_thresholds.size();
        EOS_Achievements_StatThresholds* stats = new EOS_Achievements_StatThresholds[ach->StatThresholdsCount];
        memset(stats, 0, sizeof(*stats) * ach->StatThresholdsCount);
        int i = 0;
        for (auto stat_it = stats_thresholds.begin(); stat_it != stats_thresholds.end(); ++stat_it, ++i)
        {
            stats[i].ApiVersion = EOS_ACHIEVEMENTS_STATTHRESHOLD_API_LATEST;
            try
            {
                stats[i].Name = stat_it.value()["name"].get_ref<std::string&>().c_str();
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds name for %s", it.key().c_str());
                goto achievement_error;
            }
            try
            {
                stats[i].Threshold = stat_it.value()["threshold"];
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds[\"%s\"][\"threshold\"] for %s", stat_it.key().c_str(), it.key().c_str());
                goto achievement_error;
            }
        }
        ach->StatThresholds = stats;
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds for %s", it.key().c_str());
        goto achievement_error;
    }

    *OutDefinition = ach;
    return EOS_EResult::EOS_Success;

achievement_error:
    EOS_Achievements_DefinitionV2_Release(ach);

    *OutDefinition = nullptr;
    return EOS_EResult::EOS_UnexpectedError;
}

EOS_EResult EOSSDK_Achievements::copy_unlocked_achievement(typename decltype(_unlocked_achievements)::iterator it, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    EOS_Achievements_UnlockedAchievement* ach = new EOS_Achievements_UnlockedAchievement;
    ach->ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;

    try
    {
        ach->AchievementId = (*it->second)["achievement_id"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in unlocked achievement definition: Invalid achievement_id");
        goto achievement_error;
    }
    
    try
    {
        ach->UnlockTime = (*it->second)["unlock_time"].get<int64_t>();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in unlocked achievement definition: Invalid unlock time");
        goto achievement_error;
    }

    *OutAchievement = ach;
    return EOS_EResult::EOS_Success;

achievement_error:
    delete ach;
    *OutAchievement = nullptr;
    return EOS_EResult::EOS_UnexpectedError;
}

EOS_EResult EOSSDK_Achievements::copy_player_achievement(typename decltype(_achievements)::iterator it, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    EOS_Achievements_PlayerAchievement* ach = new EOS_Achievements_PlayerAchievement;
    memset(ach, 0, sizeof(*ach));

    ach->ApiVersion = EOS_ACHIEVEMENTS_PLAYERACHIEVEMENT_API_LATEST;

    try
    {
        ach->AchievementId = it.value()["achievement_id"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid achievement_id");
        goto achievement_error;
    }
    try
    {
        ach->DisplayName = it.value()["display_name"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid display_name for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->Description = it.value()["description"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid description for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->IconURL = it.value()["icon_url"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid icon_url for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->FlavorText = it.value()["flavor_text"].get_ref<std::string&>().c_str();
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid flavor_text for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->Progress = it.value()["progress"];
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid progress for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        ach->UnlockTime = it.value()["unlock_time"];
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid unlock_time for %s", it.key().c_str());
        goto achievement_error;
    }
    try
    {
        auto& stat_info = it.value()["stat_info"];
        ach->StatInfoCount = stat_info.size();
        EOS_Achievements_PlayerStatInfo* stats = new EOS_Achievements_PlayerStatInfo[ach->StatInfoCount];
        memset(stats, 0, sizeof(*stats) * ach->StatInfoCount);
        int i = 0;
        for (auto stat_it = stat_info.begin(); stat_it != stat_info.end(); ++stat_it, ++i)
        {
            stats[i].ApiVersion = EOS_ACHIEVEMENTS_PLAYERSTATINFO_API_LATEST;
            try
            {
                stats[i].Name = stat_it.value()["name"].get_ref<std::string&>().c_str();
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid stat_info name for %s", it.key().c_str());
                goto achievement_error;
            }
            try
            {
                stats[i].CurrentValue = stat_it.value()["current_value"];
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid stat_info[\"%s\"][\"current_value\"] for %s", stats[i].Name, it.key().c_str());
                goto achievement_error;
            }
            try
            {
                stats[i].ThresholdValue = stat_it.value()["threshold_value"];
            }
            catch (...)
            {
                APP_LOG(Log::LogLevel::ERR, "Error in player achievement: Invalid stat_info[\"%s\"][\"threshold_value\"] for %s", stats[i].Name, it.key().c_str());
                goto achievement_error;
            }
        }
        ach->StatInfo = stats;
    }
    catch (...)
    {
        APP_LOG(Log::LogLevel::ERR, "Error in achievement definition: Invalid stats_thresholds for %s", it.key().c_str());
        goto achievement_error;
    }

    *OutAchievement = ach;
    return EOS_EResult::EOS_Success;

achievement_error:
    EOS_Achievements_PlayerAchievement_Release(ach);

    *OutAchievement = nullptr;
    return EOS_EResult::EOS_UnexpectedError;
}


/**
 * The following EOS_Achievements_* functions allow you to query existing achievement definitions that have been defined for your application.
 * You can also query achievement progress data for users.
 * In addition, you can also unlock one or more achievements directly.
 * You can also receive notifications when achievements are unlocked.
 */

 /**
  * Query for a list of definitions for all existing achievements, including localized text, icon IDs and whether an achievement is hidden.
  *
  * @param Options Structure containing information about the application whose achievement definitions we're retrieving.
  * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
  * @param CompletionDelegate This function is called when the query definitions operation completes.
  *
  * @return EOS_Success if the operation completes successfully
  *         EOS_InvalidParameters if any of the options are incorrect
  */
void EOSSDK_Achievements::QueryDefinitions(const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo& oqdcci = res->CreateCallback<EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);

    oqdcci.ClientData = ClientData;

    if (Options == nullptr)
    {
        oqdcci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else
    {
        oqdcci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of achievement definitions that are cached locally.
 *
 * @param Options The Options associated with retrieving the achievement definition count
 *
 * @see EOS_Achievements_CopyAchievementDefinitionByIndex
 *
 * @return Number of achievement definitions or 0 if there is an error
 */
uint32_t EOSSDK_Achievements::GetAchievementDefinitionCount(const EOS_Achievements_GetAchievementDefinitionCountOptions* Options)
{
    TRACE_FUNC();

    return _achievements_db.size();
}

/**
 * Fetches an achievement definition from a given index.
 *
 * @param Options Structure containing the index being accessed
 * @param OutDefinition The achievement definition for the given index, if it exists and is valid, use EOS_Achievements_Definition_Release when finished
 *
 * @see EOS_Achievements_DefinitionV2_Release
 *
 * @return EOS_Success if the information is available and passed out in OutDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the achievement definition is not found
 */
EOS_EResult EOSSDK_Achievements::CopyAchievementDefinitionV2ByIndex(const EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AchievementIndex >= _achievements_db.size())
    {
        *OutDefinition = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _achievements_db.begin();
    std::advance(it, Options->AchievementIndex);

    return copy_definition_v2(it, OutDefinition);
}

/**
 * Fetches an achievement definition from a given achievement ID.
 *
 * @param Options Structure containing the achievement ID being accessed
 * @param OutDefinition The achievement definition for the given achievement ID, if it exists and is valid, use EOS_Achievements_Definition_Release when finished
 *
 * @see EOS_Achievements_DefinitionV2_Release
 *
 * @return EOS_Success if the information is available and passed out in OutDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the achievement definition is not found
 */
EOS_EResult EOSSDK_Achievements::CopyAchievementDefinitionV2ByAchievementId(const EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AchievementId == nullptr)
    {
        *OutDefinition = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _achievements_db.find(Options->AchievementId);
    if (it == _achievements_db.end())
    {
        *OutDefinition = nullptr;
        return EOS_EResult::EOS_NotFound;
    }

    return copy_definition_v2(it, OutDefinition);
}

/**
 * Query for a list of achievements for a specific player, including progress towards completion for each achievement.
 *
 * @param Options Structure containing information about the player whose achievements we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate This function is called when the query player achievements operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_Achievements::QueryPlayerAchievements(const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo& oqpacci = res->CreateCallback<EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);

    oqpacci.ClientData = ClientData;
    oqpacci.UserId = Options->UserId;

    if (Options == nullptr || Options->UserId == nullptr)
    {
        oqpacci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else if(Options->UserId != GetEOS_Connect().get_myself()->first)
    {
        oqpacci.ResultCode = EOS_EResult::EOS_UnexpectedError;
    }
    else
    {
        oqpacci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of player achievements that are cached locally.
 *
 * @param Options The Options associated with retrieving the player achievement count
 *
 * @see EOS_Achievements_CopyPlayerAchievementByIndex
 *
 * @return Number of player achievements or 0 if there is an error
 */
uint32_t EOSSDK_Achievements::GetPlayerAchievementCount(const EOS_Achievements_GetPlayerAchievementCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->UserId == nullptr || Options->UserId != GetEOS_Connect().get_myself()->first)
        return 0;

    return _achievements.size();
}

/**
 * Fetches a player achievement from a given index.
 *
 * @param Options Structure containing the account id and index being accessed
 * @param OutAchievement The player achievement data for the given index, if it exists and is valid, use EOS_Achievements_PlayerAchievement_Release when finished
 *
 * @see EOS_Achievements_PlayerAchievement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutAchievement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the player achievement is not found
 */
EOS_EResult EOSSDK_Achievements::CopyPlayerAchievementByIndex(const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->UserId == nullptr || Options->AchievementIndex >= _achievements.size() || Options->UserId != GetEOS_Connect().get_myself()->first)
    {
        *OutAchievement = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _achievements.begin();
    std::advance(it, Options->AchievementIndex);

    return copy_player_achievement(it, OutAchievement);
}

/**
 * Fetches a player achievement from a given achievement id.
 *
 * @param Options Structure containing the account id and achievement id being accessed
 * @param OutAchievement The player achievement data for the given achievement id, if it exists and is valid, use EOS_Achievements_PlayerAchievement_Release when finished
 *
 * @see EOS_Achievements_PlayerAchievement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutAchievement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the player achievement is not found
 */
EOS_EResult EOSSDK_Achievements::CopyPlayerAchievementByAchievementId(const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->UserId == nullptr || Options->UserId != GetEOS_Connect().get_myself()->first)
    {
        *OutAchievement = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _achievements.find(Options->AchievementId);
    if (it == _achievements.end())
    {
        *OutAchievement = nullptr;
        return EOS_EResult::EOS_NotFound;
    }

    return copy_player_achievement(it, OutAchievement);
}

/**
 * Unlocks a number of achievements for a specific player.
 *
 * @param Options Structure containing information about the achievements and the player whose achievements we're unlocking.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate This function is called when the unlock achievements operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
void EOSSDK_Achievements::UnlockAchievements(const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (CompletionDelegate == nullptr)
        return;

    pFrameResult_t res(new FrameResult);
    EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo& ouacci = res->CreateCallback<EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo>((CallbackFunc)CompletionDelegate);

    ouacci.ClientData = ClientData;
    ouacci.UserId = Options->UserId;

    if (Options == nullptr || Options->UserId == nullptr || (Options->AchievementsCount && Options->AchievementIds == nullptr))
    {
        ouacci.AchievementsCount = 0;
        ouacci.ResultCode = EOS_EResult::EOS_InvalidParameters;
    }
    else if (Options->UserId != GetEOS_Connect().get_myself()->first)
    {
        ouacci.AchievementsCount = 0;
        ouacci.ResultCode = EOS_EResult::EOS_UnexpectedError;
    }
    else
    {
        for (int i = 0; i < Options->AchievementsCount; ++i)
        {
            if (_achievements_db.find(Options->AchievementIds[i]) != _achievements_db.end())
            {
                _achievements[Options->AchievementIds[i]]["unlock_time"] = static_cast<int64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
            }
        }

        FileManager::save_json(achievements_filename, _achievements);

        ouacci.AchievementsCount = Options->AchievementsCount;
        ouacci.ResultCode = EOS_EResult::EOS_Success;
    }

    res->done = true;
    GetCB_Manager().add_callback(this, res);
}

/**
 * Fetch the number of unlocked achievements that are cached locally.
 *
 * @param Options The Options associated with retrieving the unlocked achievement count
 *
 * @see EOS_Achievements_CopyUnlockedAchievementByIndex
 *
 * @return Number of unlocked achievements or 0 if there is an error
 */
uint32_t EOSSDK_Achievements::GetUnlockedAchievementCount(const EOS_Achievements_GetUnlockedAchievementCountOptions* Options)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->UserId != GetEOS_Connect().get_myself()->first)
        return 0;

    _unlocked_achievements.clear();
    for (auto it = _achievements.begin(); it != _achievements.end(); ++it)
    {
        try
        {
            if (it.value()["unlock_time"] != EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED)
            {
                _unlocked_achievements[it.key()] = &it.value();
            }
        }
        catch (...)
        {}
    }
    
    return _unlocked_achievements.size();
}

/**
 * Fetches an unlocked achievement from a given index.
 *
 * @param Options Structure containing the account id and index being accessed
 * @param OutAchievement The unlocked achievement data for the given index, if it exists and is valid, use EOS_Achievements_UnlockedAchievement_Release when finished
 *
 * @see EOS_Achievements_UnlockedAchievement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutAchievement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the unlocked achievement is not found
 */
EOS_EResult EOSSDK_Achievements::CopyUnlockedAchievementByIndex(const EOS_Achievements_CopyUnlockedAchievementByIndexOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->UserId != GetEOS_Connect().get_myself()->first || Options->AchievementIndex >= _unlocked_achievements.size())
    {
        *OutAchievement = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _unlocked_achievements.begin();
    std::advance(it, Options->AchievementIndex);
    
    return copy_unlocked_achievement(it, OutAchievement);
}

/**
 * Fetches an unlocked achievement from a given achievement ID.
 *
 * @param Options Structure containing the account id and achievement ID being accessed
 * @param OutAchievement The unlocked achievement data for the given achievement ID, if it exists and is valid, use EOS_Achievements_UnlockedAchievement_Release when finished
 *
 * @see EOS_Achievements_UnlockedAchievement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutAchievement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the unlocked achievement is not found
 */
EOS_EResult EOSSDK_Achievements::CopyUnlockedAchievementByAchievementId(const EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    TRACE_FUNC();
    GLOBAL_LOCK();

    if (Options == nullptr || Options->UserId != GetEOS_Connect().get_myself()->first || Options->AchievementId == nullptr)
    {
        *OutAchievement = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _unlocked_achievements.find(Options->AchievementId);

    return copy_unlocked_achievement(it, OutAchievement);
}

/**
 * Register to receive achievement unlocked notifications.
 * @note must call EOS_Achievements_RemoveNotifyAchievementsUnlocked to remove the notification
 *
 * @see EOS_Achievements_RemoveNotifyAchievementsUnlocked
 *
 * @param Options Structure containing information about the achievement unlocked notification
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param NotificationFn A callback that is fired when an achievement unlocked notification for a user has been received
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Achievements::AddNotifyAchievementsUnlocked(const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);
    EOS_Achievements_OnAchievementsUnlockedCallbackInfo& oauci = res->CreateCallback<EOS_Achievements_OnAchievementsUnlockedCallbackInfo>(CallbackFunc(NotificationFn));
    oauci.ClientData = ClientData;
    oauci.UserId = GetEOS_Connect().get_myself()->first;
    oauci.AchievementIds = nullptr;
    oauci.AchievementsCount = 0;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Register to receive achievement unlocked notifications.
 * @note must call EOS_Achievements_RemoveNotifyAchievementsUnlocked to remove the notification
 *
 * @see EOS_Achievements_RemoveNotifyAchievementsUnlocked
 *
 * @param Options Structure containing information about the achievement unlocked notification
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param NotificationFn A callback that is fired when an achievement unlocked notification for a user has been received
 *
 * @return handle representing the registered callback
 */
EOS_NotificationId EOSSDK_Achievements::AddNotifyAchievementsUnlockedV2(const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn)
{
    TRACE_FUNC();

    if (Options == nullptr || NotificationFn == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    pFrameResult_t res(new FrameResult);
    EOS_Achievements_OnAchievementsUnlockedCallbackV2Info& oauci = res->CreateCallback<EOS_Achievements_OnAchievementsUnlockedCallbackV2Info>(CallbackFunc(NotificationFn));
    oauci.ClientData = ClientData;
    oauci.UserId = GetEOS_Connect().get_myself()->first;
    oauci.UnlockTime = 0;
    oauci.AchievementId = nullptr;

    return GetCB_Manager().add_notification(this, res);
}

/**
 * Unregister from receiving achievement unlocked notifications.
 *
 * @see EOS_Achievements_AddNotifyAchievementsUnlocked
 *
 * @param InId Handle representing the registered callback
 */
void EOSSDK_Achievements::RemoveNotifyAchievementsUnlocked(EOS_NotificationId InId)
{
    TRACE_FUNC();

    GetCB_Manager().remove_notification(this, InId);
}

/**
 * DEPRECATED! Use EOS_Achievements_CopyAchievementDefinitionV2ByIndex instead.
 *
 * Fetches an achievement definition from a given index.
 *
 * @param Options Structure containing the index being accessed
 * @param OutDefinition The achievement definition for the given index, if it exists and is valid, use EOS_Achievements_Definition_Release when finished
 *
 * @see EOS_Achievements_CopyAchievementDefinitionV2ByIndex
 * @see EOS_Achievements_Definition_Release
 *
 * @return EOS_Success if the information is available and passed out in OutDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the achievement definition is not found
 */
EOS_EResult EOSSDK_Achievements::CopyAchievementDefinitionByIndex(const EOS_Achievements_CopyAchievementDefinitionByIndexOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AchievementIndex >= _achievements_db.size())
    {
        *OutDefinition = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _achievements_db.begin();
    std::advance(it, Options->AchievementIndex);

    return copy_definition(it, OutDefinition);
}

/**
 * DEPRECATED! Use EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId instead.
 *
 * Fetches an achievement definition from a given achievement ID.
 *
 * @param Options Structure containing the achievement ID being accessed
 * @param OutDefinition The achievement definition for the given achievement ID, if it exists and is valid, use EOS_Achievements_Definition_Release when finished
 *
 * @see EOS_Achievements_Definition_Release
 * @see EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId
 *
 * @return EOS_Success if the information is available and passed out in OutDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the achievement definition is not found
 */
EOS_EResult EOSSDK_Achievements::CopyAchievementDefinitionByAchievementId(const EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    TRACE_FUNC();

    if (Options == nullptr || Options->AchievementId == nullptr)
    {
        *OutDefinition = nullptr;
        return EOS_EResult::EOS_InvalidParameters;
    }

    auto it = _achievements_db.find(Options->AchievementId);
    if (it == _achievements_db.end())
    {
        *OutDefinition = nullptr;
        return EOS_EResult::EOS_NotFound;
    }

    return copy_definition(it, OutDefinition);
}

///////////////////////////////////////////////////////////////////////////////
//                                 IRunFrame                                 //
///////////////////////////////////////////////////////////////////////////////
bool EOSSDK_Achievements::CBRunFrame()
{
    return false;
}

bool EOSSDK_Achievements::RunCallbacks(pFrameResult_t res)
{
    return false;
}

void EOSSDK_Achievements::FreeCallback(pFrameResult_t res)
{
    GLOBAL_LOCK();

    switch (res->ICallback())
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
        case EOS_Achievements_OnAchievementsUnlockedCallbackInfo::k_iCallback:
        {
            EOS_Achievements_OnAchievementsUnlockedCallbackInfo& callback = res->GetCallback<EOS_Achievements_OnAchievementsUnlockedCallbackInfo>();
            // Free resources
            //for (int i = 0; i < callback.AchievementsCount; ++i)
            //{
            //    delete[] callback.AchievementIds[i];
            //}
            delete[]callback.AchievementIds;
        }

        case EOS_Achievements_OnAchievementsUnlockedCallbackV2Info::k_iCallback:
        {
            EOS_Achievements_OnAchievementsUnlockedCallbackV2Info& callback = res->GetCallback<EOS_Achievements_OnAchievementsUnlockedCallbackV2Info>();
            // Free resources
            //delete[] callback.AchievementId;
        }
        break;
    }
}

}