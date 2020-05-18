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

using namespace sdk;

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
EOS_DECLARE_FUNC(void) EOS_Achievements_QueryDefinitions(EOS_HAchievements Handle, const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
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
EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetAchievementDefinitionCount(EOS_HAchievements Handle, const EOS_Achievements_GetAchievementDefinitionCountOptions* Options)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return 0;
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
EOS_DECLARE_FUNC(void) EOS_Achievements_QueryPlayerAchievements(EOS_HAchievements Handle, const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
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
EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetPlayerAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetPlayerAchievementCountOptions* Options)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return 0;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
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
EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetUnlockedAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetUnlockedAchievementCountOptions* Options)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return 0;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByIndexOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_EResult::EOS_Success;
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
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlocked(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_INVALID_NOTIFICATIONID;
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
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_INVALID_NOTIFICATIONID;
}

/**
 * Unregister from receiving achievement unlocked notifications.
 *
 * @see EOS_Achievements_AddNotifyAchievementsUnlocked
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOS_HAchievements Handle, EOS_NotificationId InId)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByIndexOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_EResult::EOS_NotFound;
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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    TRACE_FUNC();

    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return EOS_EResult::EOS_NotFound;
}

/**
 * Release the memory associated with EOS_Achievements_DefinitionV2. This must be called on data retrieved from
 * EOS_Achievements_CopyAchievementDefinitionV2ByIndex or EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId.
 *
 * @param AchievementDefinition - The achievement definition to release.
 *
 * @see EOS_Achievements_DefinitionV2
 * @see EOS_Achievements_CopyAchievementDefinitionV2ByIndex
 * @see EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId
 */
EOS_DECLARE_FUNC(void) EOS_Achievements_DefinitionV2_Release(EOS_Achievements_DefinitionV2* AchievementDefinition)
{
    TRACE_FUNC();

    
}



/**
 * Release the memory associated with achievement definitions. This must be called on data retrieved from
 * EOS_Achievements_CopyAchievementDefinitionByIndex or EOS_Achievements_CopyAchievementDefinitionByAchievementId.
 *
 * @param AchievementDefinition - The achievement definition to release.
 *
 * @see EOS_Achievements_Definition
 * @see EOS_Achievements_CopyAchievementDefinitionByIndex
 * @see EOS_Achievements_CopyAchievementDefinitionByAchievementId
 */
EOS_DECLARE_FUNC(void) EOS_Achievements_Definition_Release(EOS_Achievements_Definition* AchievementDefinition)
{
    TRACE_FUNC();

}

/**
 * Release the memory associated with a player achievement. This must be called on data retrieved from
 * EOS_Achievements_CopyPlayerAchievementByIndex or EOS_Achievements_CopyPlayerAchievementByAchievementId.
 *
 * @param Achievement - The achievement data to release.
 *
 * @see EOS_Achievements_PlayerAchievement
 * @see EOS_Achievements_CopyPlayerAchievementByIndex
 * @see EOS_Achievements_CopyPlayerAchievementByAchievementId
 */
EOS_DECLARE_FUNC(void) EOS_Achievements_PlayerAchievement_Release(EOS_Achievements_PlayerAchievement* Achievement)
{
    TRACE_FUNC();

}

/**
 * Release the memory associated with an unlocked achievement. This must be called on data retrieved from
 * EOS_Achievements_CopyUnlockedAchievementByIndex or EOS_Achievements_CopyUnlockedAchievementByAchievementId.
 *
 * @param Achievement - The unlocked achievement data to release.
 *
 * @see EOS_Achievements_UnlockedAchievement
 * @see EOS_Achievements_CopyUnlockedAchievementByIndex
 * @see EOS_Achievements_CopyUnlockedAchievementByAchievementId
 */
EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockedAchievement_Release(EOS_Achievements_UnlockedAchievement* Achievement)
{
    TRACE_FUNC();

}