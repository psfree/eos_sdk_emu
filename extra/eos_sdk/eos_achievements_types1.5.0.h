// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_AchievementsHandle* EOS_HAchievements;

/** The most recent version of the EOS_Achievements_QueryDefinitions struct. */
#define EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_001 1

/**
 * Input parameters for the EOS_Achievements_QueryDefinitions Function.
 */
EOS_STRUCT(EOS_Achievements_QueryDefinitionsOptions001, (
	/** API Version. */
	int32_t ApiVersion;
	/** Product User ID for user who is querying definitions, if not valid default text will be returned. */
	EOS_ProductUserId UserId;
	/** Epic account ID for user who is querying definitions, if not valid default text will be returned. */
	EOS_EpicAccountId EpicUserId;
	/** An array of Achievement IDs for hidden achievements to get full details for. */
	const char** HiddenAchievementIds;
	/** The number of hidden achievements to get full details for. */
	uint32_t HiddenAchievementsCount;
));

#define EOS_ACHIEVEMENTS_STATTHRESHOLD_API_001 1
/**
 * Contains information about a collection of stat threshold data.
 *
 * The threshold will depend on the stat aggregate type:
 *   LATEST (Tracks the latest value)
 *   MAX (Tracks the maximum value)
 *   MIN (Tracks the minimum value)
 *   SUM (Generates a rolling sum of the value)
 *
 * @see EOS_Achievements_Definition
 */
EOS_STRUCT(EOS_Achievements_StatThresholds001, (
	/** API Version for the EOS_Achievements_StatThresholds struct */
	int32_t ApiVersion;
	/** The name of the stat. */
	const char* Name;
	/** The value of this data. */
	int32_t Threshold;
));

#define EOS_ACHIEVEMENTS_PLAYERSTATINFO_API_001 1
/**
 * Contains information about a collection of stat info data.
 *
 * @see EOS_Achievements_PlayerAchievement
 */
EOS_STRUCT(EOS_Achievements_PlayerStatInfo001, (
	/** API Version for the EOS_Achievements_PlayerStatInfo struct */
	int32_t ApiVersion;
	/** The name of the stat. */
	const char* Name;
	/** The current value of the stat. */
	int32_t CurrentValue;
	/** The threshold value of the stat. */
	int32_t ThresholdValue;
));

/** The most recent version of the EOS_Achievements_Definition struct. */
#define EOS_ACHIEVEMENTS_DEFINITION_API_001 1

/**
 * Contains information about a single achievement definition with localized text.
 */
EOS_STRUCT(EOS_Achievements_Definition001, (
	/** Version of the API. */
	int32_t ApiVersion;
	/** Achievement ID that can be used to uniquely identify the achievement. */
	const char* AchievementId;
	/** Text representing the Name to display in-game when achievement has been unlocked. */
	const char* DisplayName;
	/** Text representing the description to display in-game when achievement has been unlocked. */
	const char* Description;
	/** Text representing the name to display in-game when achievement is locked. */
	const char* LockedDisplayName;
	/** Text representing the description of what needs to be done to trigger the unlock of this achievement. */
	const char* LockedDescription;
	/** Text representing the description to display in-game when achievement is hidden. */
	const char* HiddenDescription;
	/** Text representing the description of what happens when the achievement is unlocked. */
	const char* CompletionDescription;
	/** Text representing the icon to display in-game when achievement is unlocked. */
	const char* UnlockedIconId;
	/** Text representing the icon to display in-game when achievement is locked. */
	const char* LockedIconId;
	/** True if achievement is hidden, false otherwise. */
	EOS_Bool bIsHidden;
	/** The number of stat thresholds. */
	int32_t StatThresholdsCount;
	/** Array of stat thresholds that need to be satisfied to unlock the achievement. */
	const EOS_Achievements_StatThresholds* StatThresholds;
));

/** The most recent version of the EOS_Achievements_GetAchievementDefinitionCount API. */
#define EOS_ACHIEVEMENTS_GETACHIEVEMENTDEFINITIONCOUNT_API_001 1

/**
 * Input parameters for the EOS_Achievements_GetAchievementDefinitionCount Function.
 */
EOS_STRUCT(EOS_Achievements_GetAchievementDefinitionCountOptions001, (
	/** Version of the API */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_Achievements_CopyAchievementDefinitionByIndexOptions struct. */
#define EOS_ACHIEVEMENTS_COPYDEFINITIONBYINDEX_API_001 1

/**
 * Input parameters for the EOS_Achievements_CopyAchievementDefinitionByIndex Function.
 */
EOS_STRUCT(EOS_Achievements_CopyAchievementDefinitionByIndexOptions001, (
	/** API Version of the EOS_Achievements_CopyAchievementDefinitionByIndexOptions function */
	int32_t ApiVersion;
	/** Index of the achievement definition to retrieve from the cache */
	uint32_t AchievementIndex;
));

/** The most recent version of the EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions struct. */
#define EOS_ACHIEVEMENTS_COPYDEFINITIONBYACHIEVEMENTID_API_001 1

/**
 * Input parameters for the EOS_Achievements_CopyAchievementDefinitionByAchievementId Function.
 */
EOS_STRUCT(EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions001, (
	/** API Version of the EOS_Achievements_CopyAchievementDefinitionByAchievementIdOptions function */
	int32_t ApiVersion;
	/** Achievement ID to look for when copying definition from the cache */
	const char* AchievementId;
));

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
EOS_DECLARE_FUNC(void) EOS_Achievements_Definition_Release(EOS_Achievements_Definition* AchievementDefinition);

/**
 * Data containing the result information for a query definitions request.
 */
EOS_STRUCT(EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo, (
	enum { k_iCallback = k_iAchievementsCallbacks + 1 };
	/** Result code for the operation. EOS_Success is returned for a successful operation, otherwise one of the error codes is returned. See eos_common.h */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Achievements_QueryDefinitions. */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Achievements_QueryDefinitions
 * @param Data A EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Achievements_OnQueryDefinitionsCompleteCallback, const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo* Data);


/** The most recent version of the EOS_Achievements_QueryPlayerAchievements struct. */
#define EOS_ACHIEVEMENTS_QUERYPLAYERACHIEVEMENTS_API_001 1

/**
 * Input parameters for the EOS_Achievements_QueryPlayerAchievements Function.
 */
EOS_STRUCT(EOS_Achievements_QueryPlayerAchievementsOptions001, (
	/** API Version. */
	int32_t ApiVersion;
	/** The account ID for the user whose achievements are to be retrieved. */
	EOS_ProductUserId UserId;
));

/** Timestamp value representing an undefined UnlockTime for EOS_Achievements_PlayerAchievement and EOS_Achievements_UnlockedAchievement */
#define EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED -1

/** The most recent version of the EOS_Achievements_PlayerAchievement struct. */
#define EOS_ACHIEVEMENTS_PLAYERACHIEVEMENT_API_001 1

/**
 * Contains information about a single player achievement.
 */
EOS_STRUCT(EOS_Achievements_PlayerAchievement001, (
	/** Version of the API. */
	int32_t ApiVersion;
	/** Achievement ID that can be used to uniquely identify the achievement. */
	const char* AchievementId;
	/** Progress towards completing this achievement (as a percentage). */
	double Progress;
	/** If not EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED then this is the POSIX timestamp that the achievement was unlocked. */
	int64_t UnlockTime;
	/** The number of player stat info entries. */
	int32_t StatInfoCount;
	/** Array of player stat info. These values can be used to calculate the overall progress towards unlocking the achievement. */
	const EOS_Achievements_PlayerStatInfo* StatInfo;
));

/** The most recent version of the EOS_Achievements_GetPlayerAchievementCount API. */
#define EOS_ACHIEVEMENTS_GETPLAYERACHIEVEMENTCOUNT_API_001 1

/**
 * Input parameters for the EOS_Achievements_GetPlayerAchievementCount Function.
 */
EOS_STRUCT(EOS_Achievements_GetPlayerAchievementCountOptions001, (
	/** Version of the API */
	int32_t ApiVersion;
	/** Account ID for the user for which to retrieve the achievement count */
	EOS_ProductUserId UserId;
));

/** The most recent version of the EOS_Achievements_CopyPlayerAchievementByIndexOptions struct. */
#define EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYINDEX_API_001 1

/**
 * Input parameters for the EOS_Achievements_CopyPlayerAchievementByIndex Function.
 */
EOS_STRUCT(EOS_Achievements_CopyPlayerAchievementByIndexOptions001, (
	/** API Version of the EOS_Achievements_CopyPlayerAchievementByIndexOptions function */
	int32_t ApiVersion;
	/** The Account ID for the user who is copying the achievement. */
	EOS_ProductUserId UserId;
	/** Index of the achievement to retrieve from the cache */
	uint32_t AchievementIndex;
));

/** The most recent version of the EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions struct. */
#define EOS_ACHIEVEMENTS_COPYPLAYERACHIEVEMENTBYACHIEVEMENTID_API_001 1

/**
 * Input parameters for the EOS_Achievements_CopyPlayerAchievementByAchievementId Function.
 */
EOS_STRUCT(EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions001, (
	/** API Version of the EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions function */
	int32_t ApiVersion;
	/** The Account ID for the user who is copying the achievement. */
	EOS_ProductUserId UserId;
	/** Achievement ID to search for when retrieving player achievement data from the cache */
	const char* AchievementId;
));

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
EOS_DECLARE_FUNC(void) EOS_Achievements_PlayerAchievement_Release(EOS_Achievements_PlayerAchievement* Achievement);

/**
 * Data containing the result information for querying a player's achievements request.
 */
EOS_STRUCT(EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo, (
	enum { k_iCallback = k_iAchievementsCallbacks + 2 };
	/** Result code for the operation. EOS_Success is returned for a successful operation, otherwise one of the error codes is returned. See eos_common.h */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Achievements_QueryPlayerAchievements. */
	void* ClientData;
	/** The Account ID of the user who initiated this request. */
	EOS_ProductUserId UserId;
));

 /**
  * Function prototype definition for callbacks passed to EOS_Achievements_QueryPlayerAchievements
  *
  * @param Data A EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo containing the output information and result
  *
  * @see EOS_Achievements_PlayerAchievement_Release
  */
EOS_DECLARE_CALLBACK(EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo* Data);


/** The most recent version of the EOS_Achievements_UnlockAchievements struct. */
#define EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_001 1

/**
 * Input parameters for the EOS_Achievements_UnlockAchievements Function.
 */
EOS_STRUCT(EOS_Achievements_UnlockAchievementsOptions001, (
	/** API Version. */
	int32_t ApiVersion;
	/** The Account ID for the user whose achievements are to be unlocked. */
	EOS_ProductUserId UserId;
	/** An array of Achievement IDs to unlock. */
	const char** AchievementIds;
	/** The number of achievements to unlock. */
	uint32_t AchievementsCount;
));

/**
 * Data containing the result information for unlocking achievements request.
 */
EOS_STRUCT(EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo, (
	enum { k_iCallback = k_iAchievementsCallbacks + 3 };
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Achievements_UnlockAchievements. */
	void* ClientData;
	/** The Account ID of the user who initiated this request. */
	EOS_ProductUserId UserId;
	/** The number of achievements to unlock. */
	uint32_t AchievementsCount;
));

/**
 * Function prototype definition for callbacks passed to EOS_Achievements_UnlockAchievements
 * @param Data A EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Achievements_OnUnlockAchievementsCompleteCallback, const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo* Data);

/** The most recent version of the EOS_Achievements_UnlockedAchievement struct. */
#define EOS_ACHIEVEMENTS_UNLOCKEDACHIEVEMENT_API_001 1

/**
 * Contains information about a single unlocked achievement.
 */
EOS_STRUCT(EOS_Achievements_UnlockedAchievement001, (
	/** Version of the API. */
	int32_t ApiVersion;
	/** Achievement ID that can be used to uniquely identify the unlocked achievement. */
	const char* AchievementId;
	/** If not EOS_ACHIEVEMENTS_ACHIEVEMENT_UNLOCKTIME_UNDEFINED then this is the POSIX timestamp that the achievement was unlocked. */
	int64_t UnlockTime;
));

/** The most recent version of the EOS_Achievements_GetUnlockedAchievementCount API. */
#define EOS_ACHIEVEMENTS_GETUNLOCKEDACHIEVEMENTCOUNT_API_001 1

/**
 * Input parameters for the EOS_Achievements_GetUnlockedAchievementCount Function.
 */
EOS_STRUCT(EOS_Achievements_GetUnlockedAchievementCountOptions001, (
	/** Version of the API */
	int32_t ApiVersion;
	/** Account ID for user for which to retrieve the unlocked achievement count */
	EOS_ProductUserId UserId;
));

/** The most recent version of the EOS_Achievements_CopyUnlockedAchievementByIndexOptions struct. */
#define EOS_ACHIEVEMENTS_COPYUNLOCKEDACHIEVEMENTBYINDEX_API_001 1

/**
 * Input parameters for the EOS_Achievements_CopyUnlockedAchievementByIndex Function.
 */
EOS_STRUCT(EOS_Achievements_CopyUnlockedAchievementByIndexOptions001, (
	/** API Version of the EOS_Achievements_CopyUnlockedAchievementByIndexOptions function */
	int32_t ApiVersion;
	/** The Account ID for the user who is copying the unlocked achievement. */
	EOS_ProductUserId UserId;
	/** Index of the unlocked achievement to retrieve from the cache */
	uint32_t AchievementIndex;
));

/** The most recent version of the EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions struct. */
#define EOS_ACHIEVEMENTS_COPYUNLOCKEDACHIEVEMENTBYACHIEVEMENTID_API_001 1

/**
 * Input parameters for the EOS_Achievements_CopyUnlockedAchievementByAchievementId Function.
 */
EOS_STRUCT(EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions001, (
	/** API Version of the EOS_Achievements_CopyUnlockedAchievementByAchievementIdOptions function */
	int32_t ApiVersion;
	/** The Account ID for the user who is copying the unlocked achievement. */
	EOS_ProductUserId UserId;
	/** AchievementId of the unlocked achievement to retrieve from the cache */
	const char* AchievementId;
));


/** The most recent version of the EOS_Achievements_AddNotifyAchievementsUnlocked API. */
#define EOS_ACHIEVEMENTS_ADDNOTIFYACHIEVEMENTSUNLOCKED_API_001 1
EOS_STRUCT(EOS_Achievements_AddNotifyAchievementsUnlockedOptions001, (
	/** Version of the API */
	int32_t ApiVersion;
));

/**
 * Output parameters for the EOS_Achievements_OnAchievementsUnlockedCallback Function.
 */
EOS_STRUCT(EOS_Achievements_OnAchievementsUnlockedCallbackInfo, (
	enum { k_iCallback = k_iAchievementsCallbacks + 4 };
	/** Context that was passed into EOS_Achievements_AddNotifyAchievementsUnlocked */
	void* ClientData;
	/** Account ID for user that received the unlocked achievements notification */
	EOS_ProductUserId UserId;
	/** The number of achievements. */
	uint32_t AchievementsCount;
	/** This member is not used and will always be set to NULL. */
	const char** AchievementIds;
));

/**
 * Function prototype definition for notifications that come from EOS_Achievements_AddNotifyAchievementsUnlocked
 *
 * @param Data A EOS_Achievements_OnAchievementsUnlockedCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Achievements_OnAchievementsUnlockedCallback, const EOS_Achievements_OnAchievementsUnlockedCallbackInfo* Data);

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
EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockedAchievement_Release(EOS_Achievements_UnlockedAchievement* Achievement);

#pragma pack(pop)
