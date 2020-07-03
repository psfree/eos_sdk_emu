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

EOS_DECLARE_FUNC(void) EOS_Achievements_QueryDefinitions(EOS_HAchievements Handle, const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    pInst->QueryDefinitions(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetAchievementDefinitionCount(EOS_HAchievements Handle, const EOS_Achievements_GetAchievementDefinitionCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->GetAchievementDefinitionCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyAchievementDefinitionV2ByIndex(Options, OutDefinition);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions* Options, EOS_Achievements_DefinitionV2** OutDefinition)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyAchievementDefinitionV2ByAchievementId(Options, OutDefinition);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_QueryPlayerAchievements(EOS_HAchievements Handle, const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;
    
    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    pInst->QueryPlayerAchievements(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetPlayerAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetPlayerAchievementCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->GetPlayerAchievementCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyPlayerAchievementByIndex(Options, OutAchievement);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement** OutAchievement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyPlayerAchievementByAchievementId(Options, OutAchievement);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    pInst->UnlockAchievements(Options, ClientData, CompletionDelegate);
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetUnlockedAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetUnlockedAchievementCountOptions* Options)
{
    if (Handle == nullptr)
        return 0;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->GetUnlockedAchievementCount(Options);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByIndexOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyUnlockedAchievementByIndex(Options, OutAchievement);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyUnlockedAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions* Options, EOS_Achievements_UnlockedAchievement** OutAchievement)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyUnlockedAchievementByAchievementId(Options, OutAchievement);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlocked(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->AddNotifyAchievementsUnlocked(Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn)
{
    if (Handle == nullptr)
        return EOS_INVALID_NOTIFICATIONID;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->AddNotifyAchievementsUnlockedV2(Options, ClientData, NotificationFn);
}

EOS_DECLARE_FUNC(void) EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOS_HAchievements Handle, EOS_NotificationId InId)
{
    if (Handle == nullptr)
        return;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    pInst->RemoveNotifyAchievementsUnlocked(InId);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByIndexOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyAchievementDefinitionByIndex(Options, OutDefinition);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions* Options, EOS_Achievements_Definition** OutDefinition)
{
    if (Handle == nullptr)
        return EOS_EResult::EOS_InvalidParameters;

    auto pInst = reinterpret_cast<EOSSDK_Achievements*>(Handle);
    return pInst->CopyAchievementDefinitionByAchievementId(Options, OutDefinition);
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

    if (AchievementDefinition == nullptr)
        return;

    //delete[] AchievementDefinition->AchievementId;
    //delete[] AchievementDefinition->FlavorText;
    //delete[] AchievementDefinition->LockedDescription;
    //delete[] AchievementDefinition->LockedDisplayName;
    //delete[] AchievementDefinition->LockedIconURL;
    //delete[] AchievementDefinition->UnlockedDescription;
    //delete[] AchievementDefinition->UnlockedDisplayName;
    //delete[] AchievementDefinition->UnlockedIconURL;
    //for (int i = 0; i < AchievementDefinition->StatThresholdsCount; ++i)
    //    delete[]AchievementDefinition->StatThresholds[i].Name;

    delete[]AchievementDefinition->StatThresholds;

    delete AchievementDefinition;
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

    if (AchievementDefinition == nullptr)
        return;

    //delete[]AchievementDefinition->AchievementId;
    //delete[]AchievementDefinition->CompletionDescription;
    //delete[]AchievementDefinition->Description;
    //delete[]AchievementDefinition->DisplayName;
    //delete[]AchievementDefinition->HiddenDescription;
    //delete[]AchievementDefinition->LockedDescription;
    //delete[]AchievementDefinition->LockedDisplayName;
    //delete[]AchievementDefinition->LockedIconId;
    //delete[]AchievementDefinition->UnlockedIconId;
    //for (int i = 0; i < AchievementDefinition->StatThresholdsCount; ++i)
    //    delete[]AchievementDefinition->StatThresholds[i].Name;

    delete[]AchievementDefinition->StatThresholds;

    delete AchievementDefinition;
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

    if (Achievement == nullptr)
        return;

    //delete[]Achievement->AchievementId;
    //delete[]Achievement->Description;
    //delete[]Achievement->DisplayName;
    //delete[]Achievement->FlavorText;
    //delete[]Achievement->IconURL;
    //for (int i = 0; i < Achievement->StatInfoCount; ++i)
    //    delete[]Achievement->StatInfo[i].Name;

    delete[]Achievement->StatInfo;

    delete Achievement;
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

    if (Achievement == nullptr)
        return;

    //delete[]Achievement->AchievementId;

    delete Achievement;
}